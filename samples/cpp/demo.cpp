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

#include <iostream>
#include <myanmartools.h>
#include <unicode/utrans.h>
#include <unicode/ustring.h>
#include <unicode/errorcode.h>
#include <unicode/translit.h>
#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <cassert>
//#include <glog/logging.h>

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
    //CHECK_LT(score1, 0.001);
    //CHECK_GT(score2, 0.999);
    std::cout.precision(6);
    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout << "Unicode Score: " << score1 << std::endl;
    std::cout << "Zawgyi Score: " << score2 << std::endl;

    UErrorCode status = U_ZERO_ERROR;
    const char16_t* id = u"Zawgyi-my";
    auto converter2 = utrans_openU(u"Zawgyi-my", -1, UTRANS_FORWARD, nullptr, -1, nullptr, &status);
    if (U_FAILURE(status)) {
        std::cout << "Failure: " << u_errorName(status);
        return -1;
    }

    const char16_t* inputZawgyi = u"အျပည္ျပည္ဆိုင္ရာ လူ႔အခြင့္အေရး ေၾကညာစာတမ္း";
    char16_t buf[256] = {0};
    
    u_strcpy(buf, inputZawgyi);
    int32_t length = -1;
    int32_t limit = u_strlen(inputZawgyi);

    utrans_transUChars(converter2, buf, &length, 256, 0, &limit, &status);

    if (U_FAILURE(status)) {
        std::cout << "Failure: " << u_errorName(status);
        return -1;
    }

    utrans_close(converter2);
    
    icu::UnicodeString output(buf);
    if (icu::UnicodeString(input1) != output) {
        std::cout << "Failed!!!" << std::endl;
    } else {
        std::cout << "Converted Text: " << output << std::endl;
    }

}
