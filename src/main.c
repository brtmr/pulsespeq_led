
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include<fftw3.h>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

#define SERIAL_DEVICE "/dev/ttyACM0"
#define DISPLAYLENGTH 16
#define DISPLAYHEIGHT 36
#define BUFSIZE 1024
#define SAMPLERATE 44100
#define MAXDB 3
#define MINDB -5
#define SLEEP_TIME 100000

double absval (fftw_complex n);
void make_window(double *window);
uint8_t db_to_display(double db);
//double average_bucket(double* data, size_t length);

const double SCALE_FACTOR = (double)(DISPLAYHEIGHT-1)/(double)(MAXDB-MINDB);
const int OUTSIZE = BUFSIZE/2 + 1;
const int MAXF = BUFSIZE/4;
const double base_freq = SAMPLERATE / BUFSIZE;
const size_t BUCKETSIZE = (BUFSIZE/4)/(DISPLAYLENGTH+1);

int main(int argc, char** argv){
    /* open the serial device */
    int serial_fd = open(SERIAL_DEVICE, O_RDWR);
    if (serial_fd == -1) {
        printf("failed to open serial device. \n");
        return -1;
    }
    //TODO: open the serial port

    /* create sample spec  */
    /* i am gonna use 44.1 kHz Stereo */
    pa_sample_spec sample_spec = {
        .format = PA_SAMPLE_FLOAT32LE,
        .rate   = SAMPLERATE,
        .channels = 1
    };

    pa_simple *stream = NULL;
    int ret=0;
    int error;
    /* Create the recording stream */
    if (!(stream = pa_simple_new(
                    NULL,
                    argv[0],
                    PA_STREAM_RECORD,
                    NULL,
                    "record",
                    &sample_spec,
                    NULL,
                    NULL,
                    &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        exit(-1);
    }

    double window[BUFSIZE];
    make_window(window);

    float buf[BUFSIZE];
    double *in;
    double *db_buf;
    fftw_complex *out;
    fftw_plan plan;
    in   = (double *) malloc(sizeof(double)*BUFSIZE);
    out  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*( OUTSIZE ));
    uint8_t signal[1];
    signal[0] = 0;

    for(;;){
        /* record new data ... */
        if (pa_simple_read(stream, buf, sizeof(buf), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            break;
        }

        /* copy it bufto a double array*/
        for (int i = 0; i < BUFSIZE; i++ ) {
            in[i] = (double) buf[i] * window[i];
        }

        /* perform fft */
        plan = fftw_plan_dft_r2c_1d( BUFSIZE , in, out, FFTW_ESTIMATE);
        fftw_execute( plan );

        /* calculate log10(abs(x)) then adjust to arduino scale */
        double sum = 0;
        for (int i = 0 ; i < MAXF ; i++) {
            sum += log10(absval(out[i]));
        }
        double db = sum / MAXF;
        signal[0] = db_to_display(db);
        /* write to arduino */
        if (write(serial_fd, signal, 1)!=1)
            break;
    }
    /* cleanup and exit */
    free(in);
    fftw_free(out);
    if (plan)
        fftw_destroy_plan( plan );
    if (stream)
        pa_simple_free(stream);
}

double absval (fftw_complex n){
    return 2*(n[0]*n[0] + n[1]*n[1]);
}

void make_window(double *window) {
    for (int i=0; i<BUFSIZE; i++) {
        window[i] = (1 - cos(i*2*M_PI / (BUFSIZE - 1)))/2;
    }
}

uint8_t db_to_display(double db){
    if (db<MINDB) return 0;
    if (db>MAXDB) return 15;
    if (db == INFINITY || db == -INFINITY || db == NAN) return 0;
    double pos = db - MINDB;
    uint8_t res = (uint8_t) (pos * SCALE_FACTOR);
    return res;
}
