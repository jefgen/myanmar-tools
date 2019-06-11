// This file shows how to use the myanmar-tools Zawgyi detector to
// detect Zawgyi text, and then how to use ICU to convert from Zawgyi
// to Unicode.

#include <cassert>
#include <iostream>

// Include the header from the myanmar-tools Zawgyi detector
#include <myanmartools.h>

// Include ICU headers
// Note: On Windows 10 version 1709 (RS3) and above you can use the following
//  #include <icu.h>
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
    // Detection ----------------------------------------------------

    // Unicode string (as UTF-8):
    const char* input1 = u8"အပြည်ပြည်ဆိုင်ရာ လူ့အခွင့်အရေး ကြေညာစာတမ်း";
    // Zawgyi string (as UTF-8):
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

    // Conversion ---------------------------------------------------

    // Note: We are only using the ICU C APIs below.

    // Unicode string (as UTF-16):
    const char16_t* unicode_16 = u"အပြည်ပြည်ဆိုင်ရာ လူ့အခွင့်အရေး ကြေညာစာတမ်း";
    // Zawgyi string (as UTF-16):
    const char16_t* zawgyi_16 = u"အျပည္ျပည္ဆိုင္ရာ လူ႔အခြင့္အေရး ေၾကညာစာတမ္း";

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
    char16_t buf[256] = { 0 };
    u_strcpy(buf, zawgyi_16);
    int32_t convertedLength = -1;
    int32_t limit = u_strlen(zawgyi_16);

    // Use the ICU Transliterator to convert from Zawgyi to Unicode.
    utrans_transUChars(converter.get(), buf, &convertedLength, 256, 0, &limit, &status);

    if (U_FAILURE(status)) {
        std::cout << "Failure: " << u_errorName(status);
        return -1;
    }

    // Verify that the converted output string matches the Unicode string.
    if (u_strcmp(unicode_16, buf) != 0) {
        std::cout << "Failed, the strings don't match!" << std::endl;
    }
    else {
        std::cout << "Success!" << std::endl;
    }
}
