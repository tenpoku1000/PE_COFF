
// Copyright (C) 2019 Shin'ichi Ichikawa. Released under the MIT license.

#include "tp_compiler.h"

static uint8_t msg_buffer[TP_MESSAGE_BUFFER_SIZE] = { 0 };

int main(int argc, char** argv)
{
    if ( ! tp_compiler(argc, argv, msg_buffer, sizeof(msg_buffer))){

        fprintf_s(stderr, "press enter key:\n");

        fflush(stderr);

        int c = getchar();

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

