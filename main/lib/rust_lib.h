#ifndef RUST_LIB_H
#define RUST_LIB_H

#include <cstdint>

extern "C" {

enum CFasterMoon {
    CFasterMoon_FirstOne = 0,
    CFasterMoon_SecondOne = 1,
};

struct CLunarData {
    std::uint64_t eclipse_time_diff_secs;
    std::uint32_t eclipse_time_diff_nanos;
    CFasterMoon faster_moon;
    std::uint64_t orbital_period_avg_secs;
    std::uint32_t orbital_period_avg_nanos;
};

enum CLightErrorCode {
    CLightErrorCode_Ok = 0,
    CLightErrorCode_InvalidLunarIdentifier = 1,
    CLightErrorCode_ModuleOverheat = 2,
    CLightErrorCode_InvalidStateError = 3,
};

CLightErrorCode light_analyzer_engage(const char *moon1, const char *moon2, CLunarData *out_data);

enum CGalacticErrorCode {
    CGalacticErrorCode_Ok = 0,
    CGalacticErrorCode_NoResult = 1,
    CGalacticErrorCode_InvalidStarIdentifier = 2,
    CGalacticErrorCode_ModuleOverheat = 3,
    CGalacticErrorCode_InvalidStateError = 4,
};

CGalacticErrorCode galactic_identifier_engage(const char *star1, const char *star2, double *distance);

struct CStarKey;
CStarKey *star_key_empty_new();
int star_key_empty_with_fuel(CStarKey *th, unsigned long long amount);
int star_key_fuelled_with_fuel(CStarKey *th, unsigned long long amount);
double star_key_fuelled_engage(CStarKey *th);
void star_key_destroy(CStarKey *th);

}

#endif // RUST_LIB_H
