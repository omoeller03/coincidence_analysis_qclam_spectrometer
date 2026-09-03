#pragma once
#include <vector>
#include <array>
#include <fstream>
#include <sstream>
#include <string>

static constexpr int N_TDC = 19;
static constexpr int N_CH  = 128;
static constexpr int N_FINE = 129;

using CalibTable = std::array<std::array<double, N_FINE>, N_CH>;
using AllCalibs  = std::array<CalibTable, N_TDC>;

inline void loadQclamCalibration(AllCalibs& calibs,
                                 const std::string& basepath)
{
    for (int tdc = 0; tdc < N_TDC; ++tdc)
    {
        char fname[512];
        sprintf(fname, "%s/tdc_%02d_calibration_table.txt",
                basepath.c_str(), tdc);

        std::ifstream f(fname);
        if (!f.good())
        {
            printf("ERROR: Cannot open calibration file %s\n", fname);
            continue;
        }

        for (int ch = 0; ch < N_CH; ++ch)
        {
            for (int fine = 0; fine < N_FINE; ++fine)
            {
                f >> calibs[tdc][ch][fine];
            }
        }

        printf("Loaded calibration for TDC %d from %s\n", tdc, fname);
    }
}
inline double computeQclamTime(uint32_t raw,
                               const CalibTable& calib)
{
    uint32_t ch      = (raw >> 21) & 0x7F;
    uint32_t clock14 = (raw >> 7)  & 0x3FFF;
    uint32_t fine7   =  raw        & 0x7F;

    uint32_t clock200 = clock14 >> 1;   // 5 ns
    uint32_t flag400  = clock14 & 1;    // 2.5 ns

    double t = 0.0;
    t += clock200 * 5.0;
    t += flag400  * 2.5;
    t += calib[ch][fine7];

    return t;
}
