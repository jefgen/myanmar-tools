/* Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cassert>
#include <iostream>
#include <myanmartools.h>

#define U_SHOW_CPLUSPLUS_API 0
#define U_DEFAULT_SHOW_DRAFT 0
#define U_HIDE_DEPRECATED_API 1
#define U_HIDE_OBSOLETE_API 1
#define U_HIDE_INTERNAL_API 1

#include <unicode/utrans.h>
#include <unicode/ustring.h>
#include <unicode/errorcode.h>
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/ustream.h>

// This is a unique pointer with a custom deleter method for cleaning up ICU opaque objects on scope exit.
template<typename T>
using deleted_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

int main() {
    // Unicode string:
    const char* input1 = u8"အပြည်ပြည်ဆိုင်ရာ လူ့အခွင့်အရေး ကြေညာစာတမ်း";
    // Zawgyi string:
    const char* input2 = u8"အျပည္ျပည္ဆိုင္ရာ လူ႔အခြင့္အေရး ေၾကညာစာတမ္း";

    // Detect that the second string is Zawgyi:
    static const auto* const detector = new google_myanmar_tools::ZawgyiDetector();
    double score1 = detector->GetZawgyiProbability(input1);
    double score2 = detector->GetZawgyiProbability(input2);

    assert(score1 < 0.001);
    assert(score2 > 0.999);
    
    std::cout.precision(6);
    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout << "Unicode Score: " << score1 << std::endl;
    std::cout << "Zawgyi Score: " << score2 << std::endl;

    // Note: We are only using the ICU C APIs below.

    // Create a ICU Transliterator object for converting from Zawygi to Unicode.
    UErrorCode status = U_ZERO_ERROR;
    deleted_unique_ptr<UTransliterator> converter(
        utrans_openU(u"Zawgyi-my", -1, UTRANS_FORWARD, nullptr, -1, nullptr, &status),
        [](UTransliterator* c) { utrans_close(c); }
    );

    if (U_FAILURE(status)) {
        std::cout << "Failed to create the Transliterator. " << u_errorName(status);
        return -1;
    }

    // Copy the Zawgyi string to a mutable buffer for conversion.
    char16_t buf[256] = {0};
    u_strcpy(buf, input2);
    int32_t convertedLength = -1;
    int32_t limit = u_strlen(input2);

    // Use the ICU Transliterator to convert from Zawgyi to Unicode.
    utrans_transUChars(converter.get(), buf, &convertedLength, 256, 0, &limit, &status);

    if (U_FAILURE(status)) {
        std::cout << "Failure: " << u_errorName(status);
        return -1;
    }
    
    // Verify that the converted output string matches the Unicode string.
    icu::UnicodeString output(buf);
    if (icu::UnicodeString(input1) != output) {
        std::cout << "Failed, the strings don't match!" << std::endl;
    } else {
        std::cout << "Success! Converted Text: " << output << std::endl;
    }
}
