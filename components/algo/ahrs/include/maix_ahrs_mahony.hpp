/*
 * Mahony AHRS
 * @author Neucrack
 * @license Apache2.0
 * @update 2016.7.1 by Neucrack, add source code.
 *         2025.7.4 by Neucrack, optimize for MaixPy/MaixCDK
 *
*/
#pragma once

#include "math.h"
#include "maix_type_vector3.hpp"
#include "maix_ahrs_type.hpp"

namespace maix::ahrs
{
    /**
      * class MahonyAHRS for Attitude Estimation from IMU data, a Complementary Filter,
      * support accelerometer, gyroscope and magnetometer fusion.
      * @maixpy maix.ahrs.MahonyAHRS
     */
    class MahonyAHRS
    {
    private:
        //! Auxiliary variables to reduce number of repeated operations
        float q0, q1, q2 , q3 ;	/** quaternion of sensor frame relative to auxiliary frame */
        float dq0, dq1, dq2 , dq3;	/** quaternion of sensor frame relative to auxiliary frame */
        float gyro_bias[3]; /** bias estimation */
        float q0q0, q0q1, q0q2, q0q3;
        float q1q1, q1q2, q1q3;
        float q2q2, q2q3;
        float q3q3;
        unsigned char bFilterInit;

    public:
        /**
          * P of PI controller, a larger P (proportional gain) leads to faster response,
          * but it increases the risk of overshoot and oscillation.
          * @maixpy maix.ahrs.MahonyAHRS.kp
          */
        float kp;
        /**
          * I of PI controller, a larger I (integral gain) helps to eliminate steady-state errors more quickly,
          * but it can accumulate error over time, potentially causing instability or slow drift.
          * @maixpy maix.ahrs.MahonyAHRS.ki
          */
        float ki;

    public:
        /**
        * class MahonyAHRS constructor.
        * @param kp P of PI controller, a larger P (proportional gain) leads to faster response,
        *           but it increases the risk of overshoot and oscillation.
        * @param ki I of PI controller, a larger I (integral gain) helps to eliminate steady-state errors more quickly,
        *           but it can accumulate error over time, potentially causing instability or slow drift.
        * @maixcdk maix.ahrs.MahonyAHRS.MahonyAHRS
        * @maixpy maix.ahrs.MahonyAHRS.__init__
        */
        MahonyAHRS(float kp,float ki)
        {
            this->kp = kp;
            this->ki = ki;
            bFilterInit = 0;
            memset(gyro_bias, 0, sizeof(gyro_bias));
        }

        /**
          * Initialize by accelerometer and magnetometer(optional).
          * If you not call this method mannually, get_angle and update method will automatically call it.
          * @param ax x axis of accelerometer, unit is g or raw data.
          * @param ay y axis of accelerometer, unit is g or raw data.
          * @param ax z axis of accelerometer, unit is g or raw data.
          * @param mx x axis of magnetometer, unit is uT or raw data, mx, my, mz all 0 means not use magnetometer.
          * @param my y axis of magnetometer, unit is uT or raw data, mx, my, mz all 0 means not use magnetometer.
          * @param mz z axis of magnetometer, unit is uT or raw data, mx, my, mz all 0 means not use magnetometer.
          * @maixpy maix.ahrs.MahonyAHRS.init
          */
        void init(float ax, float ay, float az, float mx = 0, float my = 0, float mz = 0)
        {
            float initialRoll, initialPitch;
            float cosRoll, sinRoll, cosPitch, sinPitch;
            float magX, magY;
            float initialHdg, cosHeading, sinHeading;

            initialRoll = atan2(ay, az);
            initialPitch = atan2(ax, az);

            cosRoll = cosf(initialRoll);
            sinRoll = sinf(initialRoll);
            cosPitch = cosf(initialPitch);
            sinPitch = sinf(initialPitch);

            magX = mx * cosPitch + my * sinRoll * sinPitch + mz * cosRoll * sinPitch;

            magY = my * cosRoll - mz * sinRoll;

            initialHdg = atan2f(-magY, magX);

            cosRoll = cosf(initialRoll * 0.5f);
            sinRoll = sinf(initialRoll * 0.5f);

            cosPitch = cosf(initialPitch * 0.5f);
            sinPitch = sinf(initialPitch * 0.5f);

            cosHeading = cosf(initialHdg * 0.5f);
            sinHeading = sinf(initialHdg * 0.5f);

            q0 = cosRoll * cosPitch * cosHeading + sinRoll * sinPitch * sinHeading;
            q1 = sinRoll * cosPitch * cosHeading - cosRoll * sinPitch * sinHeading;
            q2 = cosRoll * sinPitch * cosHeading + sinRoll * cosPitch * sinHeading;
            q3 = cosRoll * cosPitch * sinHeading - sinRoll * sinPitch * cosHeading;

            // auxillary variables to reduce number of repeated operations, for 1st pass
            q0q0 = q0 * q0;
            q0q1 = q0 * q1;
            q0q2 = q0 * q2;
            q0q3 = q0 * q3;
            q1q1 = q1 * q1;
            q1q2 = q1 * q2;
            q1q3 = q1 * q3;
            q2q2 = q2 * q2;
            q2q3 = q2 * q3;
            q3q3 = q3 * q3;
            bFilterInit = 1;
        }

        /**
          * Update angles by accelerometer, gyroscope and magnetometer(optional).
          * get_angle method will automatically call it.
          * @param ax x axis of accelerometer, unit is g or raw data.
          * @param ay y axis of accelerometer, unit is g or raw data.
          * @param ax z axis of accelerometer, unit is g or raw data.
          * @param ax x axis of gyroscope, unit is rad/s.
          * @param ay y axis of gyroscope, unit is rad/s.
          * @param ax z axis of gyroscope, unit is rad/s.
          * @param mx x axis of magnetometer, unit is uT or raw data, mx, my, mz all 0 means not use magnetometer.
          * @param my y axis of magnetometer, unit is uT or raw data, mx, my, mz all 0 means not use magnetometer.
          * @param mz z axis of magnetometer, unit is uT or raw data, mx, my, mz all 0 means not use magnetometer.
          * @param dt Delta time between two times call update method.
          * @maixpy maix.ahrs.MahonyAHRS.update
          */
        void update(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz, float dt)
        {
            float recipNorm;
            float halfex = 0.0f, halfey = 0.0f, halfez = 0.0f;
            float twoKp = this->kp * 2;
            float twoKi = this->ki * 2;

            // Make filter converge to initial solution faster
            // This function assumes you are in static position.
            // WARNING : in case air reboot, this can cause problem. But this is very unlikely happen.
            if(bFilterInit == 0) {
                init(ax,ay,az,mx,my,mz);
                return;
            }

            //! If magnetometer measurement is available, use it.
            if(!((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f))) {
                float hx, hy, hz, bx, bz;
                float halfwx, halfwy, halfwz;

                // Normalise magnetometer measurement
                // Will sqrt work better? PX4 system is powerful enough?
                recipNorm = 1.0 / sqrt(mx * mx + my * my + mz * mz);
                mx *= recipNorm;
                my *= recipNorm;
                mz *= recipNorm;

                // Reference direction of Earth's magnetic field
                hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) + mz * (q1q3 + q0q2));
                hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) + mz * (q2q3 - q0q1));
                hz = 2.0f * (mx * (q1q3 - q0q2) +  my * (q2q3 + q0q1) + mz * (0.5f - q1q1 - q2q2));
                bx = sqrt(hx * hx + hy * hy);
                bz = hz;

                // Estimated direction of magnetic field
                halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
                halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
                halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);

                // Error is sum of cross product between estimated direction and measured direction of field vectors
                halfex += (my * halfwz - mz * halfwy);
                halfey += (mz * halfwx - mx * halfwz);
                halfez += (mx * halfwy - my * halfwx);

            }

            //增加一个条件：  加速度的模量与G相差不远时。 0.75*G < normAcc < 1.25*G
            // Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
            if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f)))
            {
                float halfvx, halfvy, halfvz;

                // Normalise accelerometer measurement
                //归一化，得到单位加速度
                recipNorm = 1.0 / sqrt(ax * ax + ay * ay + az * az);

                ax *= recipNorm;
                ay *= recipNorm;
                az *= recipNorm;

                // Estimated direction of gravity and magnetic field
                halfvx = q1q3 - q0q2;
                halfvy = q0q1 + q2q3;
                halfvz = q0q0 - 0.5f + q3q3;

                // Error is sum of cross product between estimated direction and measured direction of field vectors
                halfex += ay * halfvz - az * halfvy;
                halfey += az * halfvx - ax * halfvz;
                halfez += ax * halfvy - ay * halfvx;
    //			DEBUG_LOG<<"\t\t\t\t\t\t"<<halfex<<"\t"<<halfey<<"\t"<<halfez<<"\n";
            }

            // Apply feedback only when valid data has been gathered from the accelerometer or magnetometer
            if(halfex != 0.0f && halfey != 0.0f && halfez != 0.0f) {
                // Compute and apply integral feedback if enabled
                if(twoKi > 0.0f) {
                    gyro_bias[0] += twoKi * halfex * dt;	// integral error scaled by Ki
                    gyro_bias[1] += twoKi * halfey * dt;
                    gyro_bias[2] += twoKi * halfez * dt;

                    // apply integral feedback
                    gx += gyro_bias[0];
                    gy += gyro_bias[1];
                    gz += gyro_bias[2];
                }
                else {
                    gyro_bias[0] = 0.0f;	// prevent integral windup
                    gyro_bias[1] = 0.0f;
                    gyro_bias[2] = 0.0f;
                }

                // Apply proportional feedback
                gx += twoKp * halfex;
                gy += twoKp * halfey;
                gz += twoKp * halfez;
    //			DEBUG_LOG<<"\t\t\t\t\t\t"<<gx<<"\t"<<gy<<"\t"<<gz<<"\n";
            }

            // Time derivative of quaternion. q_dot = 0.5*q\otimes omega.
            //! q_k = q_{k-1} + dt*\dot{q}
            //! \dot{q} = 0.5*q \otimes P(\omega)
            dq0 = 0.5f*(-q1 * gx - q2 * gy - q3 * gz);
            dq1 = 0.5f*(q0 * gx + q2 * gz - q3 * gy);
            dq2 = 0.5f*(q0 * gy - q1 * gz + q3 * gx);
            dq3 = 0.5f*(q0 * gz + q1 * gy - q2 * gx);

            q0 += dt*dq0;
            q1 += dt*dq1;
            q2 += dt*dq2;
            q3 += dt*dq3;

            // Normalise quaternion
            recipNorm = 1.0 / sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
            q0 *= recipNorm;
            q1 *= recipNorm;
            q2 *= recipNorm;
            q3 *= recipNorm;

            // Auxiliary variables to avoid repeated arithmetic
            q0q0 = q0 * q0;
            q0q1 = q0 * q1;
            q0q2 = q0 * q2;
            q0q3 = q0 * q3;
            q1q1 = q1 * q1;
            q1q2 = q1 * q2;
            q1q3 = q1 * q3;
            q2q2 = q2 * q2;
            q2q3 = q2 * q3;
            q3q3 = q3 * q3;
        }

        /**
          * Get angle by mahony complementary filter, will automatically call update method,
          * and automatically call init in first time.
          * @param acc accelerometer data, unit is g or raw data. maix.vector.Vector3f type.
          * @param gyro gyroscope data, unit can be rad/s or degree/s, if rad/s, arg radian should be true. maix.vector.Vector3f type.
          * @param mag magnetometer data, optional, if no magnetometer, set all value to 0. maix.vector.Vector3f type.
          * @param dt delta T of two time call get_anle, unit is second, float type.
          * @param radian if gyro's unit is rad/s, set this arg to true, degree/s set to false.
          * @return rotation angle data, maix.vector.Vector3f type.
          * @maixpy maix.ahrs.MahonyAHRS.get_angle
          */
        tensor::Vector3f get_angle(tensor::Vector3f acc, tensor::Vector3f gyro, tensor::Vector3f mag, float dt, bool radian = false)
        {
            tensor::Vector3f angle;

            update(acc.x, acc.y, acc.z, gyro.x, gyro.y, gyro.z, mag.x, mag.y, mag.z, dt);

            if(radian)
            {
                angle.y = asin(-2 * q1 * q3 + 2 * q0* q2); // pitch for x axis in front, or roll
                angle.x = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1);
                angle.z = atan2(2*(q0*q3+q1*q2), 1 - 2 * (q2*q2+q3*q3));
            }
            else
            {
                angle.y = asin(-2 * q1 * q3 + 2 * q0* q2)* RAD2DEG; // pitch for x axis in front, or roll
                angle.x = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* RAD2DEG;
                angle.z = atan2(2*(q0*q3+q1*q2), 1 - 2 * (q2*q2+q3*q3)) * RAD2DEG;
            }

            return angle;
        }

        /**
          * reset to not initialized status.
          * @maixpy maix.ahrs.MahonyAHRS.reset
          */
        void reset()
        {
            bFilterInit = 0;
        }

    };
}; //namespace maix::ahrs
