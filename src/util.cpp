
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <string>
#include <valarray>
#include <vector>

#include "util.hpp"

/*
    Restrict a value to between two limits.
*/
double clamp(double value, double minimum, double maximum) {
    if (value < minimum)
        return minimum;
    else if (value > maximum)
        return maximum;
    else
        return value;
}

/*
    Flatten a 2D valarray into a 1D vector. Useful for Catch2 test comparisons.
*/
std::vector<double> flatten(std::valarray<std::valarray<double>>& array) {
    std::vector<double> vector;

    for (int i_row = 0; i_row < array.size(); i_row++) {
        for (int i_col = 0; i_col < array[i_row].size(); i_col++) {
            vector.push_back(array[i_row][i_col]);
        }
    }

    return vector;
}

/*
    Neatly print a 1D array.
*/
void print_array(std::valarray<double>& array) {
    int n_col = array.size();

    printf("[");
    for (int i_col = 0; i_col < n_col; ++i_col) {
        printf("%g", array[i_col]);
        if (i_col != n_col - 1) printf(", ");
    }
    printf("]\n");

    return;
}

/*
    Neatly print a 1D array as 2D with n_col columns (2nd dimension).
*/
void print_array_2D(std::valarray<double>& array, int n_col) {
    int n_tot = array.size();
    int n_row = n_tot / n_col;

    printf("[");
    for (int i_row = 0; i_row < n_row; ++i_row) {
        if (i_row == 0)
            printf("[");
        else
            printf(" [");
        for (int i_col = 0; i_col < n_col; ++i_col) {
            printf("%g", array[i_row * n_col + i_col]);
            if (i_col != n_col - 1)
                printf(", ");
            else if (i_row != n_row - 1)
                printf("]\n");
            else
                printf("]]\n");
        }
    }

    return;
}

/*
    Neatly print an actual 2D array.
*/
void print_array_2D(std::valarray<std::valarray<double>>& array) {
    int n_row = array.size();
    int n_col;

    printf("[");
    for (int i_row = 0; i_row < n_row; ++i_row) {
        n_col = array[i_row].size();

        if (i_row == 0)
            printf("[");
        else
            printf(" [");

        for (int i_col = 0; i_col < n_col; ++i_col) {
            printf("%g", array[i_row][i_col]);
            if (i_col != n_col - 1)
                printf(", ");
            else if (i_row != n_row - 1)
                printf("]\n");
            else
                printf("]]\n");
        }
    }

    return;
}

/*
    Basic equivalent of numpy.arange().
*/
std::valarray<double> arange(double start, double stop, double step) {
    // Create the array more easily as a vector
    std::vector<double> tmp_array;
    for (double value = start; value < stop; value += step) tmp_array.push_back(value);

    // Convert to a valarray
    std::valarray<double> array(tmp_array.data(), tmp_array.size());
    return array;
}

/*
    Calculate the number of elapsed seconds between two times.
*/
double gettimelapsed(struct timeval start, struct timeval end) {
    double seconds;
    double microseconds;

    seconds = end.tv_sec - start.tv_sec;
    microseconds = end.tv_usec - start.tv_usec;

    if (microseconds < 0.0) {
        seconds -= 1.0;
        microseconds = 1e6 - microseconds;
    }

    microseconds /= 1e6;

    return seconds + microseconds;
}

/*
    Load a 2D image from a text file.
    
    File contents:
        n_rows  n_columns
        row_0_column_0  row_0_column_1  ...  row_0_column_n
        row_1_column_0  ...             ...  ...
        ...             ...             ...  ...
        row_n_column 0  ...             ...
    
    Parameters
    ----------
    filename : str
        The path to the file to load.
        
    Returns
    -------
    image : std::valarray<std::valarray<double>>
        The loaded 2D image array.
*/
std::valarray<std::valarray<double>> load_image_from_txt(char* filename) {
    FILE* f = fopen(filename, "r");
    if (f == NULL) error("Failed to open image file '%s'", filename);

    // Load image dimensions
    int n_rows;
    int n_columns;
    int c = fscanf(f, "%d %d", &n_rows, &n_columns);
    if (c != 2) error("Failed to read n_rows, n_columns '%s'", filename);

    // Load image data
    std::valarray<std::valarray<double>> image(
        std::valarray<double>(n_columns), n_rows);
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            c = fscanf(f, "%lf", &image[i_row][i_col]);
            if (c != 1)
                error("Failed to read image [%d, %d] '%s'", i_row, i_col, filename);
        }
    }

    fclose(f);

    return image;
}

/*
    Save a 2D image to a text file.
    
    File contents:
        n_rows  n_columns
        row_0_column_0  row_0_column_1  ...  row_0_column_n
        row_1_column_0  ...             ...  ...
        ...             ...             ...  ...
        row_n_column 0  ...             ...
    
    Parameters
    ----------
    filename : str
        The path to the file to load.
        
    image : std::valarray<std::valarray<double>>
        The 2D image array to save.
*/
void save_image_to_txt(char* filename, std::valarray<std::valarray<double>> image) {
    FILE* f = fopen(filename, "w");
    if (f == NULL) error("Failed to open file '%s'", filename);

    // Save image dimensions
    int n_rows = image.size();
    int n_columns = image[0].size();
    fprintf(f, "%d %d \n", n_rows, n_columns);

    // Save image data
    for (int i_row = 0; i_row < n_rows; i_row++) {
        for (int i_col = 0; i_col < n_columns; i_col++) {
            fprintf(f, "%lf ", image[i_row][i_col]);
        }
        fprintf(f, "\n");
    }

    fclose(f);

    return;
}
