//
// Copyright 2015 Ettus Research LLC
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <shd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHD_TEST_EXECUTE_OR_GOTO(label, ...) \
    if(__VA_ARGS__){ \
        fprintf(stderr, "Error occurred at %s:%d\n", __FILE__, (__LINE__-1)); \
        return_code = EXIT_FAILURE; \
        goto label; \
    }

#define BUFFER_SIZE 1024

int main(){

    // Variables
    int return_code;
    shd_mboard_eeprom_handle mb_eeprom;
    shd_dboard_eeprom_handle db_eeprom;
    int db_revision;
    char str_buffer[BUFFER_SIZE];

    return_code = EXIT_SUCCESS;

    /*
     * Motherboard EEPROM test
     */

    // Create EEPROM handle
    SHD_TEST_EXECUTE_OR_GOTO(end_of_test,
        shd_mboard_eeprom_make(&mb_eeprom)
    )

    // Set a value, retrieve it, and make sure it matches
    SHD_TEST_EXECUTE_OR_GOTO(free_mboard_eeprom,
        shd_mboard_eeprom_set_value(
            mb_eeprom,
            "serial",
            "F12345"
        )
    )
    SHD_TEST_EXECUTE_OR_GOTO(free_mboard_eeprom,
        shd_mboard_eeprom_get_value(
            mb_eeprom,
            "serial",
            str_buffer,
            BUFFER_SIZE
        )
    )
    if(strcmp(str_buffer, "F12345")){
        return_code = EXIT_FAILURE;
        fprintf(stderr, "%s:%d: Mismatched EEPROM value: \"%s\" vs. \"F12345\"\n",
                        __FILE__, __LINE__,
                        str_buffer);
        goto free_mboard_eeprom;
    }

    /*
     * Daughterboard EEPROM test
     */

    // Create EEPROM handle
    SHD_TEST_EXECUTE_OR_GOTO(free_mboard_eeprom,
        shd_dboard_eeprom_make(&db_eeprom)
    )

    // Set the ID, retrieve it, and make sure it matches
    SHD_TEST_EXECUTE_OR_GOTO(free_dboard_eeprom,
        shd_dboard_eeprom_set_id(db_eeprom, "0x0067")
    )
    SHD_TEST_EXECUTE_OR_GOTO(free_dboard_eeprom,
        shd_dboard_eeprom_get_id(
            db_eeprom,
            str_buffer,
            BUFFER_SIZE
        )
    )
    if(strcmp(str_buffer, "0x0067")){
        return_code = EXIT_FAILURE;
        fprintf(stderr, "%s:%d: Mismatched daughterboard ID: \"%s\" vs. \"0x0067\"\n",
                        __FILE__, __LINE__,
                        str_buffer);
        goto free_dboard_eeprom;
    }

    // Set the serial, retrieve it, and make sure it matches
    SHD_TEST_EXECUTE_OR_GOTO(free_dboard_eeprom,
        shd_dboard_eeprom_set_serial(db_eeprom, "F12345")
    )
    SHD_TEST_EXECUTE_OR_GOTO(free_dboard_eeprom,
        shd_dboard_eeprom_get_serial(
            db_eeprom,
            str_buffer,
            BUFFER_SIZE
        )
    )
    if(strcmp(str_buffer, "F12345")){
        return_code = EXIT_FAILURE;
        fprintf(stderr, "%s:%d: Mismatched daughterboard serial: \"%s\" vs. \"F12345\"\n",
                        __FILE__, __LINE__,
                        str_buffer);
        goto free_dboard_eeprom;
    }

    // Set the revision, retrieve it, and make sure it matches
    SHD_TEST_EXECUTE_OR_GOTO(free_dboard_eeprom,
        shd_dboard_eeprom_set_revision(db_eeprom, 4)
    )
    SHD_TEST_EXECUTE_OR_GOTO(free_dboard_eeprom,
        shd_dboard_eeprom_get_revision(db_eeprom, &db_revision)
    )
    if(db_revision != 4){
        return_code = EXIT_FAILURE;
        fprintf(stderr, "%s:%d: Mismatched daughterboard revision: \"%d\" vs. 4\n",
                        __FILE__, __LINE__, db_revision);
        goto free_dboard_eeprom;
    }

    free_dboard_eeprom:
        if(return_code){
            shd_dboard_eeprom_last_error(db_eeprom, str_buffer, BUFFER_SIZE);
            fprintf(stderr, "db_eeprom error: %s\n", str_buffer);
        }
	shd_dboard_eeprom_free(&db_eeprom);

    free_mboard_eeprom:
        if(return_code){
            shd_mboard_eeprom_last_error(mb_eeprom, str_buffer, BUFFER_SIZE);
            fprintf(stderr, "mb_eeprom error: %s\n", str_buffer);
        }
	shd_mboard_eeprom_free(&mb_eeprom);

    end_of_test:
        if(!return_code){
            printf("\nNo errors detected\n");
        }
        return return_code;
}
