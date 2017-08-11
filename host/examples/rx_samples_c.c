/*
 * Copyright 2015 Ettus Research LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <shd.h>

#include "getopt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXECUTE_OR_GOTO(label, ...) \
    if(__VA_ARGS__){ \
        return_code = EXIT_FAILURE; \
        goto label; \
    }

void print_help(void){
    fprintf(stderr, "rx_samples_c - A simple RX example using SHD's C API\n\n"

                    "Options:\n"
                    "    -a (device args)\n"
                    "    -f (frequency in Hz)\n"
                    "    -r (sample rate in Hz)\n"
                    "    -g (gain)\n"
                    "    -n (number of samples to receive)\n"
                    "    -o (output filename, default = \"out.dat\")\n"
                    "    -v (enable verbose prints)\n"
                    "    -h (print this help message)\n");
};

int main(int argc, char* argv[])
{
    if(shd_set_thread_priority(shd_default_thread_priority, true)){
        fprintf(stderr, "Unable to set thread priority. Continuing anyway.\n");
    }

    int option = 0;
    double freq = 500e6;
    double rate = 1e6;
    double gain = 5.0;
    char* device_args = "";
    size_t channel = 0;
    char* filename = "out.dat";
    size_t n_samples = 1000000;
    bool verbose = false;
    int return_code = EXIT_SUCCESS;
    bool custom_filename = false;
    char error_string[512];

    // Process options
    while((option = getopt(argc, argv, "a:f:r:g:n:o:vh")) != -1){
        switch(option){
            case 'a':
                device_args = strdup(optarg);
                break;

            case 'f':
                freq = atof(optarg);
                break;

            case 'r':
                rate = atof(optarg);
                break;

            case 'g':
                gain = atof(optarg);
                break;

            case 'n':
                n_samples = atoi(optarg);
                break;

            case 'o':
                filename = strdup(optarg);
                custom_filename = true;
                break;

            case 'v':
                verbose = true;
                break;

            case 'h':
                print_help();
                goto free_option_strings;

            default:
                print_help();
                return_code = EXIT_FAILURE;
                goto free_option_strings;
        }
    }

    // Create SMINI
    shd_smini_handle smini;
    fprintf(stderr, "Creating SMINI with args \"%s\"...\n", device_args);
    EXECUTE_OR_GOTO(free_option_strings,
        shd_smini_make(&smini, device_args)
    )

    // Create RX streamer
    shd_rx_streamer_handle rx_streamer;
    EXECUTE_OR_GOTO(free_smini,
        shd_rx_streamer_make(&rx_streamer)
    )

    // Create RX metadata
    shd_rx_metadata_handle md;
    EXECUTE_OR_GOTO(free_rx_streamer,
        shd_rx_metadata_make(&md)
    )

    // Create other necessary structs
    shd_tune_request_t tune_request = {
        .target_freq = freq,
        .rf_freq_policy = SHD_TUNE_REQUEST_POLICY_AUTO,
        .dsp_freq_policy = SHD_TUNE_REQUEST_POLICY_AUTO,
    };
    shd_tune_result_t tune_result;

    shd_stream_args_t stream_args = {
        .cpu_format = "fc32",
        .otw_format = "sc16",
        .args = "",
        .channel_list = &channel,
        .n_channels = 1
    };

    shd_stream_cmd_t stream_cmd = {
        .stream_mode = SHD_STREAM_MODE_NUM_SAMPS_AND_DONE,
        .num_samps = n_samples,
        .stream_now = true
    };

    size_t samps_per_buff;
    float *buff = NULL;
    void **buffs_ptr = NULL;
    FILE *fp = NULL;
    size_t num_acc_samps = 0;

    // Set rate
    fprintf(stderr, "Setting RX Rate: %f...\n", rate);
    EXECUTE_OR_GOTO(free_rx_metadata,
        shd_smini_set_rx_rate(smini, rate, channel)
    )

    // See what rate actually is
    EXECUTE_OR_GOTO(free_rx_metadata,
        shd_smini_get_rx_rate(smini, channel, &rate)
    )
    fprintf(stderr, "Actual RX Rate: %f...\n", rate);

    // Set gain
    fprintf(stderr, "Setting RX Gain: %f dB...\n", gain);
    EXECUTE_OR_GOTO(free_rx_metadata,
        shd_smini_set_rx_gain(smini, gain, channel, "")
    )

    // See what gain actually is
    EXECUTE_OR_GOTO(free_rx_metadata,
        shd_smini_get_rx_gain(smini, channel, "", &gain)
    )
    fprintf(stderr, "Actual RX Gain: %f...\n", gain);

    // Set frequency
    fprintf(stderr, "Setting RX frequency: %f MHz...\n", freq/1e6);
    EXECUTE_OR_GOTO(free_rx_metadata,
        shd_smini_set_rx_freq(smini, &tune_request, channel, &tune_result)
    )

    // See what frequency actually is
    EXECUTE_OR_GOTO(free_rx_metadata,
        shd_smini_get_rx_freq(smini, channel, &freq)
    )
    fprintf(stderr, "Actual RX frequency: %f MHz...\n", freq / 1e6);

    // Set up streamer
    stream_args.channel_list = &channel;
    EXECUTE_OR_GOTO(free_rx_streamer,
        shd_smini_get_rx_stream(smini, &stream_args, rx_streamer)
    )

    // Set up buffer
    EXECUTE_OR_GOTO(free_rx_streamer,
        shd_rx_streamer_max_num_samps(rx_streamer, &samps_per_buff)
    )
    fprintf(stderr, "Buffer size in samples: %zu\n", samps_per_buff);
    buff = malloc(samps_per_buff * 2 * sizeof(float));
    buffs_ptr = (void**)&buff;

    // Issue stream command
    fprintf(stderr, "Issuing stream command.\n");
    EXECUTE_OR_GOTO(free_buffer,
        shd_rx_streamer_issue_stream_cmd(rx_streamer, &stream_cmd)
    )

    // Set up file output
    fp = fopen(filename, "wb");

    // Actual streaming
    while (num_acc_samps < n_samples) {
        size_t num_rx_samps = 0;
        EXECUTE_OR_GOTO(close_file,
            shd_rx_streamer_recv(rx_streamer, buffs_ptr, samps_per_buff, &md, 3.0, false, &num_rx_samps)
        )

        shd_rx_metadata_error_code_t error_code;
        EXECUTE_OR_GOTO(close_file,
            shd_rx_metadata_error_code(md, &error_code)
        )
        if(error_code != SHD_RX_METADATA_ERROR_CODE_NONE){
            fprintf(stderr, "Error code 0x%x was returned during streaming. Aborting.\n", return_code);
            goto close_file;
        }

        // Handle data
        fwrite(buff, sizeof(float) * 2, num_rx_samps, fp);
        if (verbose) {
            time_t full_secs;
            double frac_secs;
            shd_rx_metadata_time_spec(md, &full_secs, &frac_secs);
            fprintf(stderr, "Received packet: %zu samples, %.f full secs, %f frac secs\n",
                    num_rx_samps,
                    difftime(full_secs, (time_t) 0),
                    frac_secs);
        }

        num_acc_samps += num_rx_samps;
    }

    // Cleanup
    close_file:
        fclose(fp);

    free_buffer:
        if(buff){
            if(verbose){
                fprintf(stderr, "Freeing buffer.\n");
            }
            free(buff);
        }
        buff = NULL;
        buffs_ptr = NULL;

    free_rx_streamer:
        if(verbose){
            fprintf(stderr, "Cleaning up RX streamer.\n");
        }
        shd_rx_streamer_free(&rx_streamer);

    free_rx_metadata:
        if(verbose){
            fprintf(stderr, "Cleaning up RX metadata.\n");
        }
        shd_rx_metadata_free(&md);

    free_smini:
        if(verbose){
            fprintf(stderr, "Cleaning up SMINI.\n");
        }
        if(return_code != EXIT_SUCCESS && smini != NULL){
            shd_smini_last_error(smini, error_string, 512);
            fprintf(stderr, "SMINI reported the following error: %s\n", error_string);
        }
        shd_smini_free(&smini);

    free_option_strings:
        if(strcmp(device_args,"")){
            free(device_args);
        }
        if(custom_filename){
            free(filename);
        }

    fprintf(stderr, (return_code ? "Failure\n" : "Success\n"));
    return return_code;
}
