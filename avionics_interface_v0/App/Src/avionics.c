/**
 * @file    avionics.c
 * @brief   Sensor bring-up, sampling loop and radio telemetry.
 */
#include "avionics.h"
#include <stdio.h>
#include "main.h"
#include "spi.h"
#include "sdio.h"
#include "bmp388.h"
#include "bno085.h"
#include "rfm95.h"

#define GROUND_SAMPLES 25U
#define PRINT_PERIOD_MS 1000U

avionics_telemetry_t g_telemetry;

static bmp388_t baro;
static bno085_t imu;
static rfm95_t radio;
static uint8_t status;
static float ground_pressure_pa = 101325.0f;
static uint32_t last_tx_ms;
static uint32_t last_print_ms;
static uint32_t packets_sent;
static bmp388_data_t last_baro;

static int16_t to_fixed(float v, float scale)
{
  float x = v * scale;
  if (x > 32767.0f) {
    return 32767;
  }
  if (x < -32768.0f) {
    return -32768;
  }
  return (int16_t)x;
}

/* Average a second of pressure readings on the pad as the AGL reference. */
static void calibrate_ground_pressure(void)
{
  float sum = 0.0f;
  uint32_t n = 0;
  uint32_t start = HAL_GetTick();
  bmp388_data_t d;

  while (n < GROUND_SAMPLES && HAL_GetTick() - start < 2000U) {
    if (bmp388_data_ready(&baro) && bmp388_read(&baro, &d) == HAL_OK) {
      sum += d.pressure_pa;
      n++;
    }
  }
  if (n > 0U) {
    ground_pressure_pa = sum / (float)n;
  }
}

static const char *ok_or_failed(uint8_t bit)
{
  return (status & bit) != 0U ? "OK" : "FAILED";
}

/* Boot report on the serial port: one line per part, so a wiring problem
 * shows up straight away. */
static void print_startup_report(void)
{
  printf("\r\n=== DSRT avionics started ===\r\n");
  printf("  BMP388 barometer : %s\r\n", ok_or_failed(AVIONICS_STATUS_BMP388));
  printf("  BNO085 IMU       : %s\r\n", ok_or_failed(AVIONICS_STATUS_BNO085));
  printf("  RFM95 radio      : %s\r\n", ok_or_failed(AVIONICS_STATUS_RFM95));
  printf("  SD card          : %s\r\n",
         (status & AVIONICS_STATUS_SD) != 0U ? "found" : "not found");
  printf("  status byte      : 0x%02X\r\n\r\n", status);
}

/* One human-readable line of the latest readings. Takes ~10 ms at 115200
 * baud, so it only runs once a second. */
static void print_readings(uint32_t now)
{
  printf("[%5lu.%03lu s]", now / 1000U, now % 1000U);
  if ((status & AVIONICS_STATUS_BMP388) != 0U) {
    printf("  baro %.0f Pa  alt %.2f m  %.1f C",
           (double)last_baro.pressure_pa, (double)g_telemetry.altitude_agl_m,
           (double)last_baro.temperature_c);
  }
  if ((status & AVIONICS_STATUS_BNO085) != 0U) {
    printf("  |  accel %.2f %.2f %.2f m/s2  gyro %.2f %.2f %.2f rad/s",
           (double)imu.data.accel_mps2.x, (double)imu.data.accel_mps2.y,
           (double)imu.data.accel_mps2.z, (double)imu.data.gyro_rps.x,
           (double)imu.data.gyro_rps.y, (double)imu.data.gyro_rps.z);
  }
  if ((status & AVIONICS_STATUS_RFM95) != 0U) {
    printf("  |  radio %lu sent", packets_sent);
  }
  printf("\r\n");
}

void avionics_init(void)
{
  /* Send printf() output straight away instead of waiting for a full buffer. */
  setvbuf(stdout, NULL, _IONBF, 0);
  printf("\r\nDSRT avionics booting (takes a few seconds)...\r\n");
  status = 0;

  if (bmp388_init(&baro, &hspi1, BMP388_CS_GPIO_Port, BMP388_CS_Pin) == HAL_OK) {
    status |= AVIONICS_STATUS_BMP388;
    calibrate_ground_pressure();
  }

  if (bno085_init(&imu, &hspi1, BNO085_CS_GPIO_Port, BNO085_CS_Pin,
                  BNO085_INT_GPIO_Port, BNO085_INT_Pin,
                  BNO085_RST_GPIO_Port, BNO085_RST_Pin,
                  BNO085_WAKE_GPIO_Port, BNO085_WAKE_Pin) == HAL_OK &&
      bno085_enable_report(&imu, BNO085_REPORT_ROTATION_VECTOR, AVIONICS_IMU_INTERVAL_US) == HAL_OK &&
      bno085_enable_report(&imu, BNO085_REPORT_ACCELEROMETER, AVIONICS_IMU_INTERVAL_US) == HAL_OK &&
      bno085_enable_report(&imu, BNO085_REPORT_GYROSCOPE, AVIONICS_IMU_INTERVAL_US) == HAL_OK) {
    status |= AVIONICS_STATUS_BNO085;
  }

  if (rfm95_init(&radio, &hspi2, RFM95_CS_GPIO_Port, RFM95_CS_Pin,
                 RFM95_RST_GPIO_Port, RFM95_RST_Pin,
                 RFM95_G0_GPIO_Port, RFM95_G0_Pin,
                 AVIONICS_RADIO_FREQ_MHZ, AVIONICS_RADIO_TX_DBM) == HAL_OK) {
    status |= AVIONICS_STATUS_RFM95;
  }

  /* The SD card is optional: without one the rest of the system still runs. */
  if (SD_Card_Init() == HAL_OK) {
    status |= AVIONICS_STATUS_SD;
  }

  g_telemetry.status = status;
  last_tx_ms = HAL_GetTick();
  last_print_ms = last_tx_ms;
  print_startup_report();
}

void avionics_run(void)
{
  avionics_telemetry_t *t = &g_telemetry;

  if ((status & AVIONICS_STATUS_BNO085) != 0U && bno085_service(&imu)) {
    const bno085_data_t *d = &imu.data;
    t->quat[0] = to_fixed(d->rotation.i, 16384.0f);
    t->quat[1] = to_fixed(d->rotation.j, 16384.0f);
    t->quat[2] = to_fixed(d->rotation.k, 16384.0f);
    t->quat[3] = to_fixed(d->rotation.real, 16384.0f);
    t->accel[0] = to_fixed(d->accel_mps2.x, 256.0f);
    t->accel[1] = to_fixed(d->accel_mps2.y, 256.0f);
    t->accel[2] = to_fixed(d->accel_mps2.z, 256.0f);
    t->gyro[0] = to_fixed(d->gyro_rps.x, 512.0f);
    t->gyro[1] = to_fixed(d->gyro_rps.y, 512.0f);
    t->gyro[2] = to_fixed(d->gyro_rps.z, 512.0f);
  }

  if ((status & AVIONICS_STATUS_BMP388) != 0U && bmp388_data_ready(&baro)) {
    bmp388_data_t d;
    if (bmp388_read(&baro, &d) == HAL_OK) {
      last_baro = d;
      t->pressure_pa = d.pressure_pa;
      t->altitude_agl_m = bmp388_altitude_m(d.pressure_pa, ground_pressure_pa);
      t->temperature_cc = to_fixed(d.temperature_c, 100.0f);
    }
  }

  if ((status & AVIONICS_STATUS_RFM95) != 0U) {
    uint32_t now = HAL_GetTick();
    if (!rfm95_poll(&radio) && now - last_tx_ms >= AVIONICS_TELEMETRY_PERIOD_MS) {
      last_tx_ms = now;
      t->time_ms = now;
      t->status = status;
      if (rfm95_send(&radio, (const uint8_t *)t, sizeof(*t)) == HAL_OK) {
        packets_sent++;
      }
    }
  }

  uint32_t now = HAL_GetTick();
  if (now - last_print_ms >= PRINT_PERIOD_MS) {
    last_print_ms = now;
    print_readings(now);
  }
}
