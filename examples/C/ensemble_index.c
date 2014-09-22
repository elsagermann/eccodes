/*
 * Copyright 2005-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities granted to it by
 * virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.
 */

/*
 * Description: index a GRIB file, select 2t and compute ensemble mean.
 *
 */

#include "grib_api.h"

int main(int argc, char * argv[])
{
    int ret;
    int i, j;
    int count=0;
    size_t paramIdSize, numberSize, values_len;
    char** paramId;
    long* number;
    double* values;
    double* result=NULL;
    double min=1000,max=0,avg=0;
    grib_index* index;
    grib_handle* h=NULL;

    /* create index of file contents for paramId and number */
    index = grib_index_new_from_file(0, "eps", "paramId,number",&ret);
    GRIB_CHECK(ret,0);

    /* get size of "paramId" list */
    GRIB_CHECK(grib_index_get_size(index, "paramId", &paramIdSize),0);
    printf("grib contains %ld different parameters\n",paramIdSize);
    /* allocate memory for "paramId" list */
    paramId = (char**) malloc(paramIdSize * sizeof(char*));
    /* get list of "paramId" */
    GRIB_CHECK(grib_index_get_string(index, "paramId", paramId, &paramIdSize),0);

    /* get size of ensemble number list */
    GRIB_CHECK(grib_index_get_size(index, "number", &numberSize),0);
    printf("grib contains %ld different ensemble members\n",numberSize);
    /* allocate memory for ensemble number list */
    number = (long*) malloc(numberSize * sizeof(long));
    /* get list of ensemble numbers */
    GRIB_CHECK(grib_index_get_long(index, "number", number, &numberSize),0);

    /* select T850 with paramId 130 */
    GRIB_CHECK(grib_index_select_string(index, "paramId", "130"), 0);

    /* loop over all members */
    for (i = 0; i < numberSize; i++) {
        count++;
        /* select an individual ensemble number */
        GRIB_CHECK(grib_index_select_long(index, "number", number[i]), 0);

        /* create handle for next grib message */
        h=grib_handle_new_from_index(index, &ret);
        if (ret) {
            printf("error: %d\n", ret);
            exit(ret);
        }

        /* get the size of the values array */
        GRIB_CHECK(grib_get_size(h, "values", &values_len), 0);

        /* allocate memory for the grib message */
        values = malloc(values_len * sizeof(double));

        /* allocate memory for result */
        if ( i == 0 ) {
            result = (double *)calloc(values_len, sizeof(double));
        }

        /* get data values */
        GRIB_CHECK(grib_get_double_array(h, "values", values, &values_len), 0);

        /* add up values */
        for (j = 0; j < values_len; j++)
            result[j] = result[j] + values[j];

        /* free memory and grib message handle */
        free(values);
        GRIB_CHECK(grib_handle_delete(h), 0);
    }

    printf("We considered %d ensemble members\n", count);

    for (j = 0; j < values_len; j++)
        result[j] = result[j] / count;

    for (j = 0; j < values_len; j++) {
        if (min > result[j]) {
            min = result[j];
        }
        if (max < result[j]) {
            max = result[j];
        }
        avg += result[j];
    }
    avg = avg/values_len;

    printf("==============================================================================\n");
    printf("Stats for ensemble mean of T850\n");
    printf("Min: %f Max: %f Avg: %f\n",min,max,avg);
    printf("==============================================================================\n");

    /* finally free all other memory */
    for (i=0;i<paramIdSize;i++)
        free(paramId[i]);
    free(paramId);
    free(number);
    free(result);
    grib_index_delete(index);

    return 0;
}
