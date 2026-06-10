# Generates a .cpp file embedding a binary file as a byte array.
# Usage: cmake -DINPUT=<file> -DOUTPUT=<file.cpp> -DSYMBOL=<name> -P embed_resource.cmake
# Emits: const unsigned char <SYMBOL>[]; const std::size_t <SYMBOL>Size;

if(NOT INPUT OR NOT OUTPUT OR NOT SYMBOL)
    message(FATAL_ERROR "embed_resource.cmake requires INPUT, OUTPUT and SYMBOL")
endif()

file(READ "${INPUT}" hex_data HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," byte_list "${hex_data}")
# Wrap lines for readability / compiler friendliness
string(REGEX REPLACE "(0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,)" "\\1\n" byte_list "${byte_list}")

get_filename_component(input_name "${INPUT}" NAME)
file(WRITE "${OUTPUT}" "// Auto-generated from ${input_name} by embed_resource.cmake — do not edit.
#include <cstddef>

extern const unsigned char ${SYMBOL}[];
extern const std::size_t ${SYMBOL}Size;

const unsigned char ${SYMBOL}[] = {
${byte_list}
};
const std::size_t ${SYMBOL}Size = sizeof(${SYMBOL});
")
