/**************************************************************************************
Filename:       ir_main.c
Revised:        Date: 2016-11-05
Revision:       Revision: 1.0

Description:    This file provides main entry for irda decoder

Revision log:
* 2016-11-05: created by strawmanbobi
**************************************************************************************/

#pragma ide diagnostic ignored "OCUnusedMacroInspection"

#include <ctype.h>
#include <libgen.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/ir_defs.h"
#include "include/ir_decode.h"

#define INPUT_MAX 3

// global variable definition
t_remote_ac_status ac_status;
UINT16 user_data[USER_DATA_SIZE];

void input_number(int *val)
{
    char n[50]={0};
    int i = 0;
    *val = 0;
    scanf("%s", n);
    getchar();
    while(1)
    {
        if(n[i] < '0'||n[i] > '9')
        {
            printf("\nInvalid number format, please re-input : ");
            scanf("%s", n);
            i=0;
        }
        else
        {
            i++;
        }
        if(n[i] == '\0')
            break;
    }
    i = 0;
    while (n[i] != '\0')
    {
        *val = (*val * 10 + (int)n[i] - 48);
        i++;
    }
}

static INT8 decode_as_ac(char *file_name)
{
    BOOL op_match = TRUE;
    BOOL change_wind_dir = FALSE;
    UINT8 function_code = AC_FUNCTION_MAX;
    int key_code = 0;
    int first_time = 1;
    int length = 0;
    int index = 0;

    // get status
    UINT8 supported_mode = 0x00;
    INT8 min_temperature = 0;
    INT8 max_temperature = 0;
    UINT8 supported_speed = 0x00;
    UINT8 supported_swing = 0x00;
    UINT8 supported_wind_direction = 0x00;

    BOOL need_control;

    // init air conditioner status
    ac_status.ac_display = 0;
    ac_status.ac_sleep = 0;
    ac_status.ac_timer = 0;
    ac_status.ac_power = AC_POWER_OFF;
    ac_status.ac_mode = AC_MODE_COOL;
    ac_status.ac_temp = AC_TEMP_20;
    ac_status.ac_swing = AC_SWING_ON;
    ac_status.ac_wind_speed = AC_WS_AUTO;

    if (IR_DECODE_FAILED == ir_file_open(REMOTE_CATEGORY_AC, 0, file_name))
    {
        ir_close();
        return IR_DECODE_FAILED;
    }

    do
    {
        if (1 == first_time)
        {
            printf("Please input valid key code "
                   "(Key code could be referenced from https://irext.net/doc#keymap) : \n");
            first_time = 0;
        }
        else
        {
            printf("Please input valid key code : \n");
        }
        input_number(&key_code);

        op_match = TRUE;
        need_control = FALSE;
        change_wind_dir = FALSE;

        if (99 == key_code)
        {
            break;
        }

        if (14 == key_code)
        {
            if (IR_DECODE_SUCCEEDED == get_supported_mode(&supported_mode))
            {
                printf("supported mode = %02X\n", supported_mode);
            }
            else
            {
                printf("get supported mode failed\n");
            }
        }
        else if (15 == key_code)
        {
            if (IR_DECODE_SUCCEEDED == get_supported_wind_speed(ac_status.ac_mode, &supported_speed))
            {
                printf("supported wind speed in %d = %02X\n", ac_status.ac_mode, supported_speed);
            }
            else
            {
                printf("get supported wind speed failed\n");
            }
        }
        else if (16 == key_code)
        {
            if (IR_DECODE_SUCCEEDED == get_temperature_range(ac_status.ac_mode, &min_temperature, &max_temperature))
            {
                printf("supported temperature range in mode %d = %d, %d\n",
                          ac_status.ac_mode, min_temperature, max_temperature);
            }
            else
            {
                printf("get supported temperature range failed\n");
            }
        }
        else if (17 == key_code)
        {
            if (IR_DECODE_SUCCEEDED == get_supported_wind_direction(&supported_wind_direction))
            {
                printf("supported swing type = %02X\n", supported_wind_direction);
            }
            else
            {
                printf("get swing type failed\n");
            }
        }
        else
        {
            switch (key_code)
            {
                case 0:
                    ac_status.ac_power = ((ac_status.ac_swing == AC_POWER_ON) ? AC_POWER_OFF : AC_POWER_ON);
                    need_control = TRUE;
                    break;

                case 1:
                    ++ac_status.ac_mode;
                    ac_status.ac_mode = ac_status.ac_mode % AC_MODE_MAX;
                    need_control = TRUE;
                    break;

                case 2:
                case 7:
                    ac_status.ac_temp = ((ac_status.ac_temp == AC_TEMP_30) ? AC_TEMP_30 : (ac_status.ac_temp + 1));
                    need_control = TRUE;
                    break;

                case 3:
                case 8:
                    ac_status.ac_temp = ((ac_status.ac_temp == AC_TEMP_16) ? AC_TEMP_16 : (ac_status.ac_temp - 1));
                    need_control = TRUE;
                    break;

                case 9:
                    ++ac_status.ac_wind_speed;
                    ac_status.ac_wind_speed = ac_status.ac_wind_speed % AC_WS_MAX;
                    need_control = TRUE;
                    break;

                case 10:
                    ac_status.ac_swing = ((ac_status.ac_swing == AC_SWING_ON) ? AC_SWING_OFF : AC_SWING_ON);
                    need_control = TRUE;
                    break;

                case 11:
                    if (ac_status.ac_swing == AC_SWING_OFF) {
                        change_wind_dir = TRUE;
                    }
                    need_control = TRUE;
                    break;

                default:
                    op_match = FALSE;
                    break;
            }

            if (TRUE == op_match && TRUE == need_control)
            {
                printf("switch AC to power = %d, mode = %d, temp = %d, speed = %d, swing = %d with function code = %d\n",
                       ac_status.ac_power,
                       ac_status.ac_mode,
                       ac_status.ac_temp,
                       ac_status.ac_wind_speed,
                       ac_status.ac_swing,
                       function_code);

                length = ir_decode(function_code, user_data, &ac_status, change_wind_dir);
                printf("\n === Binary decoded : %d\n", length);
                for (index = 0; index < length; index++)
                {
                    printf("%d, ", user_data[index]);
                }
                printf("===\n");
            }
        }
    } while (TRUE);

    ir_close();

    return IR_DECODE_SUCCEEDED;
}

static INT8 decode_as_tv(char *file_name, UINT8 ir_hex_encode)
{
    // keyboard input
    int key_code = 0;
    int first_time = 1;
    int length = 0;
    int index = 0;

    // here remote category TV represents for command typed IR code
    if (IR_DECODE_FAILED == ir_file_open(REMOTE_CATEGORY_TV, ir_hex_encode, file_name))
    {
        ir_close();
        return IR_DECODE_FAILED;
    }

    do
    {
        if (1 == first_time)
        {
            printf("Please input valid key code "
                   "(Key code could be referenced from https://irext.net/doc#keymap) : \n");
            first_time = 0;
        }
        else
        {
            printf("Please input valid key code : \n");
        }
        input_number(&key_code);
        if (99 == key_code)
        {
            break;
        }
        length = ir_decode(key_code, user_data, NULL, 0);
        printf("\n === Binary decoded : %d\n", length);
        for (index = 0; index < length; index++)
        {
            printf("%d, ", user_data[index]);
        }
        printf("===\n");

    } while (TRUE);

    ir_close();
    return IR_DECODE_SUCCEEDED;
}

static void print_usage(const char *progn) {
    printf("Usage: %s [function] [file] [subcate]\n"
              "[function] : 0 - decode for AC; 1 - decode for TV\n"
              "[file]     : the remote control binary file\n"
              "[subcate]  : the sub_cate value from remote_index", progn);
}

int main(int argc, char *argv[])
{
    char function = '0';
    UINT8 ir_hex_encode = 0;

    if (4 != argc)
    {
        print_usage(basename(argv[0]));
        return -1;
    }

    function = argv[1][0];
    ir_hex_encode = (UINT8) (argv[3][0] - '0');

    switch (function)
    {
        case '0':
            printf("Decode %s as status-typed binary\n", argv[2]);
            decode_as_ac(argv[2]);
            break;

        case '1':
            printf("Decode %s as command-typed binary in sub_cate %d\n", argv[2], ir_hex_encode);
            decode_as_tv(argv[2], ir_hex_encode);
            break;

        default:
            printf("Decode functionality not supported : %c\n", function);
            break;
    }


}