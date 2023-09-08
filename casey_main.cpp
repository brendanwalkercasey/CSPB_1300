/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Brendan Casey

- All project requirements fully met? (YES or NO):
    NO

- If no, please explain what you could not get to work: 

    I was unable to get the change image option to work (option 0)...it compiles, but I was unsuccessful in implimenting an option to change the input image (i.e. cout << "0) Change image (current: " << image_file << ")\n"; does not return the "new" image, should the user decide to change the input image). 
    
- Did you do any optional enhancements? If so, please explain:

    I added a nested escape command "(cin.fail())" for process 2,5,6,8, and 9 which allows the user to "Quit" the process, and return to the main menu if a letter instead of a number is entered.
    
    I also added "#include <iomanip>" so I could use the test function provided by Dr. Jones (though the "set w" to set field width wasnt necessary, it was nice to keep in the test function, so I decided to add "#include <iomanip>").  
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <iomanip> //Brendan added
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

/*
//My Function Template

vector<vector<Pixel>> process_number(const vector<vector<Pixel>>& image)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                //[operation]:
                //PYTHON--> 
                

                //[operation]
                //PYTHON--> 

                //[operation]
                //PYTHON-->
                
                
                

				// Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = new_red;
                new_image[row][col].green = new_green;
                new_image[row][col].blue = new_blue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
        
}
int main()
{
    
   // Read in BMP image file into a 2D vector (using read_image function)
    vector<vector<Pixel>> image = read_image("sample.bmp");
    
	// Call process function using the input 2D vector and save the result returned to a new 2D vector
    vector<vector<Pixel>> transform_image = process_1(image);
    
	// Write the resulting 2D vector to a new BMP image file (using write_image function)
    bool success = write_image("transform.bmp", transform_image);

    cout << "\n\n\nThis line should be your own code!\n\n\n" << endl;

    return 0;
}
*/


//............................
// Process1
//............................
vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                //Distance towards center:
                //PYTHON--> "distance = math.sqrt((col - width/2)**2+(row - height/2)**2)"
                
                float first = (col-num_columns/2.0) * (col-num_columns/2.0);
                float second = (row-num_rows/2.0) * (row-num_rows/2.0);
                float distance = sqrt(first + second);
                 
                /*
                //Doesn't work...
                int delta_y = col-num_columns/2;
                int delta_x = row-num_rows/2;
                float distance = sqrt(pow(delta_y,2.0) + pow(delta_x,2.0));
                
                //Also doesnt work...
                float first = col-num_columns;
                float second = (first/2) * (first/2);
                float third = row-num_rows;
                float fourth = (fourth/2) * (fourth/2);
                float distance = sqrt(second + second);
                */

                //Multiply by scaling factor
                //PYTHON--> "scaling_factor = (height - distance)/height"
                double scaling_factor = (num_rows - distance)/num_rows;

                //Transform image
                //PYTHON--> "newred = p.getRed()*scaling_factor"
                //PYTHON--> "newgreen = p.getGreen()*scaling_factor"
                //PYTHON--> "newblue = p.getBlue()*scaling_factor"
                double new_red = red_color * scaling_factor;
                double new_green = green_color * scaling_factor;
                double new_blue = blue_color * scaling_factor;
                
                // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = new_red;
                new_image[row][col].green = new_green;
                new_image[row][col].blue = new_blue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
}

//............................
// Process2
//............................
vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image, double scaling_factor)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                //Scaling Factor, and other values initialized
                //double scaling_factor = 0.3;
                double newred = 0;
                double newgreen = 0;
                double newblue = 0;
                //Average RBB values
                double average_value = (red_color + green_color + blue_color)/3.0;
                
                //Transform image (If Pixel is light, make it lighter)
                if (average_value >= 170)
                {
                    newred = ((int)255 -(255 - red_color) * scaling_factor);
                    newgreen = ((int)255 -(255 - green_color) * scaling_factor);
                    newblue = ((int)255 -(255 - blue_color) * scaling_factor);
                }
                else if (average_value < 90)
                {
                    newred = red_color*scaling_factor;
                    newgreen = green_color*scaling_factor;
                    newblue = blue_color*scaling_factor;
                }
                else
                {
                    newred = red_color;
                    newgreen = green_color;
                    newblue = blue_color;
                }

				// Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = newred;
                new_image[row][col].green = newgreen;
                new_image[row][col].blue = newblue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
        

}

//............................
// Process3
//............................
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                // Values initialized
                double newred = 0;
                double newgreen = 0;
                double newblue = 0;
                //Average values tp get grey value
                double grey_value = (red_color + green_color + blue_color)/3.0;
                
                //Transform image (set all colors to grey)
                newred = grey_value;
                newgreen = grey_value;
                newblue = grey_value;

				// Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = newred;
                new_image[row][col].green = newgreen;
                new_image[row][col].blue = newblue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
        

}

//............................
// Process4
//............................
vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the with REVERSED size as the input 2D vector
        vector<vector<Pixel>> new_image(num_columns, vector<Pixel> (num_rows));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector.  Perform the operation on the color values (refer to Runestone for this) and Transform image (rotate 90 degrees clockwise):

                int red_color = image[(num_rows-1)-row][col].red;
                int green_color = image[(num_rows-1)-row][col].green;
                int blue_color = image[(num_rows-1)-row][col].blue;
               

				// Save the new color values to the corresponding pixel located at this COLUMN and ROW (instead of row/column)in the new 2D vector:
                
                new_image[col][row].red = red_color;
                new_image[col][row].green = green_color;
                new_image[col][row].blue = blue_color;
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;  

}

//............................
// Process5
//............................
vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number)

    {
        int angle = number * 90;
        if (angle % 90 != 0)
        {
            cout << "angle must be a multiple of 90 degrees.";
            return image;
        }
        else if (angle % 360 == 0)
        {
            return image;
        }
        else if (angle % 360 == 90)
        {
            return process_4(image);
            //return process_4(const vector<vector<Pixel>>& image);
            //return vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image);
        }
        else if (angle % 360 == 180)
        {
            return process_4(process_4(image));
            //return process_4(process_4(const vector<vector<Pixel>>& image));
            //return vector<vector<Pixel>> process_4(vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image));
        }
        else
        {
            return process_4(process_4(process_4(image)));
            //return process_4(process_4(process_4(const vector<vector<Pixel>>& image)));
            //return vector<vector<Pixel>> process_4(vector<vector<Pixel>> process_4(vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image)));
        }
    }
//............................
// Process6
//............................
vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale)
{
    // Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(x_scale*num_rows, vector<Pixel> (y_scale*num_columns));

        // For each of the rows in the input 2D vector
        for (int row = 0; row < (x_scale*num_rows); row++)
        { 
            // For each of the columns in the input 2D vector
            for (int col = 0; col < (y_scale*num_columns); col++)
            {
            // Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row/x_scale][col/y_scale].red;
                int green_color = image[row/x_scale][col/y_scale].green;
                int blue_color = image[row/x_scale][col/y_scale].blue;
                // Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = red_color;
                new_image[row][col].green = green_color;
                new_image[row][col].blue = blue_color;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
        


}

//............................
// Process7
//............................
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                // Values initialized
                double newred = 0;
                double newgreen = 0;
                double newblue = 0;
                //Average values tp get grey value
                double grey_value = (red_color + green_color + blue_color)/3.0;
                
                //Transform image (Set all values to black and white (high contrast) using thresholds)
                if (grey_value >= 255/2)
                {
                    newred = 255;
                    newgreen = 255;
                    newblue = 255;
                }
                else
                {
                    newred = 0;
                    newgreen = 0;
                    newblue = 0;
                }
                

				// Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = newred;
                new_image[row][col].green = newgreen;
                new_image[row][col].blue = newblue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
        

}

//............................
// Process8
//............................
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                //Values initialized
                double newred = 0;
                double newgreen = 0;
                double newblue = 0;
                
                //Transform image (If Pixel is light, make it lighter)
              
                newred = ((int)255 -(255 - red_color) * scaling_factor);
                newgreen = ((int)255 -(255 - green_color) * scaling_factor);
                newblue = ((int)255 -(255 - blue_color) * scaling_factor);
                

				// Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = newred;
                new_image[row][col].green = newgreen;
                new_image[row][col].blue = newblue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
        

}

//............................
// Process9
//............................
vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor)

	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                //Values initialized
                double newred = 0;
                double newgreen = 0;
                double newblue = 0;
                
                //Transform image (If Pixel is light, make it lighter)
              
                newred = red_color * scaling_factor;
                newgreen = green_color * scaling_factor;
                newblue = blue_color * scaling_factor;
                

				// Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = newred;
                new_image[row][col].green = newgreen;
                new_image[row][col].blue = newblue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
        

}


//............................
// Process10
//............................
vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image)
	{
		// Get the number of rows/columns from the input 2D vector (remember: num_rows is height, num_columns is width)
        int num_rows = image.size();
        int num_columns = image[0].size();

		// Define a new 2D vector the same size as the input 2D vector
        vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));

		// For each of the rows in the input 2D vector
        for (int row = 0; row < num_rows; row++)
        { 
			// For each of the columns in the input 2D vector
            for (int col = 0; col < num_columns; col++)
            {
				// Get the color values for the pixel located at this row and column in the input 2D vector
                int red_color = image[row][col].red;
                int green_color = image[row][col].green;
                int blue_color = image[row][col].blue;
                

				// Perform the operation on the color values (refer to Runestone for this)
                
                //Max Values
                
                //Initial variables (needed for Max Option 1 and Max Option 2)
                int color_vals[3] = {red_color, green_color, blue_color};
                int k;
                int max = color_vals[0];
                /*
                //Option 1...Doesnt Work, function not allowed error...
                int findMax(int a[], int n)
                {
                    for (k = 0; k < n; k++)
                    {
                        if (a[k] > max)
                        {
                            max = a[k];
                        }
                    }
                    return max;
                }
                int max_color = findMax(color_vals[3], 3)
                */
                
                // Option 2 (iterate to find max value in color_val array)
                for (k = 1; k < 3; k++)
                {
                    if (color_vals[k] > max)
                    {
                        max = color_vals[k];
                    }
                }
                
                /*
                //Option 3 // Also works!
                int max = 0;
                if (red_color > green_color && red_color > blue_color)
                {
                    max = red_color;
                }
                else if (green_color > red_color && green_color > blue_color)
                {
                    max = green_color;
                }
                else
                {
                    max = blue_color;
                }
                */
                
                //other initialized values
                int max_color = max;
                double newred = 0;
                double newgreen = 0;
                double newblue = 0;
                
                //Transform image (converts image to only black, white, red, blue, and green)
                if (red_color + green_color + blue_color >= 550)
                {
                    newred = 255;
                    newgreen = 255;
                    newblue = 255;
                }
                else if (red_color + green_color + blue_color <= 150)
                {
                    newred = 0;
                    newgreen = 0;
                    newblue = 0;
                }
                else if (max_color == red_color)
                {
                    newred = 255;
                    newgreen = 0;
                    newblue = 0;
                }
                else if (max_color == green_color)
                {
                    newred = 0;
                    newgreen = 255;
                    newblue = 0;
                }
                else
                {
                    newred = 0;
                    newgreen = 0;
                    newblue = 255;
                }
                

				// Save the new color values to the corresponding pixel located at this row and column in the new 2D vector
                new_image[row][col].red = newred;
                new_image[row][col].green = newgreen;
                new_image[row][col].blue = newblue;
                
            }
        }
		// Return the new 2D vector after the nested for loop is complete
	    return new_image;
} 


int main()
{
    // User can enter an input BMP filename
    cout << "\n\n\nCSPB 1300 Image Processing Application! \nEnter input BMP filename (including extension, e.g. .bmp): ";
    string image_file;
    cin >> image_file;
    
    bool done = false;
    while (!done)
    {
    // Displays a menu of available selections to the user
    cout << "\n\nIMAGE PROCESSING MENU:\n\n";
        //One selection to change the input image if desired
        cout << "0) Change image (current: " << image_file << ")\n";
        //One for each of the image processing functions
        cout << "1) Vignette\n";
        cout << "2) Clarendon\n";
        cout << "3) Grayscale\n";
        cout << "4) Rotate 90 degrees\n";
        cout << "5) Rotate multiple 90 degrees\n";
        cout << "6) Enlarge\n";
        cout << "7) High contrast\n";
        cout << "8) Lighten\n";
        cout << "9) Darken\n";
        cout << "10) Black, white, red, green, blue\n";
        cout << "\n\nEnter menu selection (Q to quit): ";
        int response;
        cin >> response;
        
            
        // Ask for user input until user is finished
        if (cin.fail())
        {
            cin.clear();
            string clearing_item;
            cin >> clearing_item;
            done = true;
            cout << "\n\nExiting Program...See you next time!\n\n";
        }
        
        else if (response == 0)
        {
            //System Response
            cout << "Changing image file. Please enter new image:";
            string image_file;
            cin >> image_file; 

        }
        else if (response == 1)
        {
            //System Response
            cout << "Performing Vignette on " << image_file << endl;
            cout << "\n\nEnter the output file name (also including extension): ";
            string output_file;
            cin >> output_file;
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(image_file);
            // Call process function using the input 2D vector and save the result returned to a new 2D vector
            vector<vector<Pixel>> transform_image = process_1(image);
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool success = write_image(output_file, transform_image);
            cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl;
        }
        else if (response == 2)
        {
            //System Response
            cout << "\n\nEnter scaling factor: ";
            double scale_fac;
            cin >> scale_fac;
            if (cin.fail())
            {
                cin.clear();
                string clearing_item;
                cin >> clearing_item;
                cout << "\n\nInvalid Response, Exiting Process 2 and Returning to Main Menu\n\n";
            }
            else
            {
                //System Response
                cout << "Performing Clarendon on " << image_file << " with scaling factor " << scale_fac << endl;
                cout << "\n\nEnter the output file name (also including extension): ";
            string output_file;
            cin >> output_file;
                // Read in BMP image file into a 2D vector (using read_image function)
                vector<vector<Pixel>> image = read_image(image_file);
                // Call process function using the input 2D vector and save the result returned to a new 2D vector
                vector<vector<Pixel>> transform_image = process_2(image, scale_fac);
                // Write the resulting 2D vector to a new BMP image file (using write_image function)
                bool success = write_image(output_file, transform_image);
                cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
            }
        }    
        else if (response == 3)
        {
            //System Response
            cout << "Performing Grayscale on " << image_file << endl;
            cout << "\n\nEnter the output file name (also including extension): ";
            string output_file;
            cin >> output_file;
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(image_file);
            // Call process function using the input 2D vector and save the result returned to a new 2D vector
            vector<vector<Pixel>> transform_image = process_3(image);
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool success = write_image(output_file, transform_image);
            cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
        }
        else if (response == 4)
        {
            //System Response
            cout << "Transposing image 90 degrees clockwise on " << image_file << endl;
            cout << "\n\nEnter the output file name (also including extension): ";
            string output_file;
            cin >> output_file;
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(image_file);
            // Call process function using the input 2D vector and save the result returned to a new 2D vector
            vector<vector<Pixel>> transform_image = process_4(image);
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool success = write_image(output_file, transform_image);
            cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
            
        }
        else if (response == 5)
        {
            //System Response
            cout << "\n\nEnter number of 90 degree rotations (again type Q to quit): ";
            int rotate_num;
            cin >> rotate_num;
            if (cin.fail())
            {
                cin.clear();
                string clearing_item;
                cin >> clearing_item;
                cout << "\n\nExiting Process 5, and Returning to Main Menu\n\n";
            }
            else
            {
                cout << "Transposing image 90 degrees clockwise on " << image_file << " " << rotate_num << " times!" << endl;
                cout << "\n\nEnter the output file name (also including extension): ";
                string output_file;
                cin >> output_file;
                // Read in BMP image file into a 2D vector (using read_image function)
                vector<vector<Pixel>> image = read_image(image_file);
                // Call process function using the input 2D vector and save the result returned to a new 2D vector
                vector<vector<Pixel>> transform_image = process_5(image, rotate_num);
                // Write the resulting 2D vector to a new BMP image file (using write_image function)
                bool success = write_image(output_file, transform_image);
                cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
        
            }
        }
        else if (response == 6)
        {
            //System Response
            cout << "\n\nEnter x-scale: ";
            int xscale_num;
            cin >> xscale_num;
            cout << "\n\nEnter y-scale: ";
            int yscale_num;
            cin >> yscale_num;
            if (cin.fail())
            {
                cin.clear();
                string clearing_item;
                cin >> clearing_item;
                cout << "\n\nInvalid Response, Exiting Process 6 and Returning to Main Menu\n\n";
            }
            else
            {
                cout << "Enlarging image " << image_file << endl;
                cout << "\n\nEnter the output file name (also including extension): ";
                string output_file;
                cin >> output_file;
                // Read in BMP image file into a 2D vector (using read_image function)
                vector<vector<Pixel>> image = read_image(image_file);
                // Call process function using the input 2D vector and save the result returned to a new 2D vector
                vector<vector<Pixel>> transform_image = process_6(image, xscale_num, yscale_num);
                // Write the resulting 2D vector to a new BMP image file (using write_image function)
                bool success = write_image(output_file, transform_image);
                cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
        
            }
        }
        else if (response == 7)
        {
            //System Response
            cout << "Performing High contrast on " << image_file << endl;
            cout << "\n\nEnter the output file name (also including extension): ";
            string output_file;
            cin >> output_file;
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(image_file);
            // Call process function using the input 2D vector and save the result returned to a new 2D vector
            vector<vector<Pixel>> transform_image = process_7(image);
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool success = write_image(output_file, transform_image);
            cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
        }
        else if (response == 8)
        {
            //System Response
            cout << "\n\nEnter scaling factor: ";
            double scale_fac;
            cin >> scale_fac;
            if (cin.fail())
            {
                cin.clear();
                string clearing_item;
                cin >> clearing_item;
                cout << "\n\nInvalid Response, Exiting Process 8 and Returning to Main Menu\n\n";
            }
            else
            {
                cout << "Performing Lighten on " << image_file << " with scaling factor " << scale_fac << endl;
                cout << "\n\nEnter the output file name (also including extension): ";
                string output_file;
                cin >> output_file;
                // Read in BMP image file into a 2D vector (using read_image function)
                vector<vector<Pixel>> image = read_image(image_file);
                // Call process function using the input 2D vector and save the result returned to a new 2D vector
                vector<vector<Pixel>> transform_image = process_8(image, scale_fac);
                // Write the resulting 2D vector to a new BMP image file (using write_image function)
                bool success = write_image(output_file, transform_image);
                cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl;
            }
            
        }
        else if (response == 9)
        {
            //System Response
            cout << "\n\nEnter scaling factor: ";
            double scale_fac;
            cin >> scale_fac;
            if (cin.fail())
            {
                cin.clear();
                string clearing_item;
                cin >> clearing_item;
                cout << "\n\nInvalid Response, Exiting Process 9 and Returning to Main Menu\n\n";
            }
            else
            {
                cout << "Performing Darken on " << image_file << endl;
                cout << "\n\nEnter the output file name (also including extension): ";
                string output_file;
                cin >> output_file;
                // Read in BMP image file into a 2D vector (using read_image function)
                vector<vector<Pixel>> image = read_image(image_file);
                // Call process function using the input 2D vector and save the result returned to a new 2D vector
                vector<vector<Pixel>> transform_image = process_9(image, scale_fac);
                // Write the resulting 2D vector to a new BMP image file (using write_image function)
                bool success = write_image(output_file, transform_image);
                cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
            }
        }
        else if (response == 10)
        {
            //System Response
            cout << "Performing Black, white, red, green, blue transform on " << image_file << endl;
            cout << "\n\nEnter the output file name (also including extension): ";
            string output_file;
            cin >> output_file;
            // Read in BMP image file into a 2D vector (using read_image function)
            vector<vector<Pixel>> image = read_image(image_file);
            // Call process function using the input 2D vector and save the result returned to a new 2D vector
            vector<vector<Pixel>> transform_image = process_10(image);
            // Write the resulting 2D vector to a new BMP image file (using write_image function)
            bool success = write_image(output_file, transform_image);
            cout << "\n\n\n" << output_file << " has been created!\n\n\n" << endl; 
        }
    }
    return 0;
}

//************************************** Testing block **********************************
/*
// Below is a sample 2D vector and loop program to test output for each process against expected results (results detailed on applied.cs.colorado.edu).

int main()
{
    vector<vector<Pixel>> tiny =
    {
        {{  0,  5, 10},{ 15, 20, 25},{ 30, 35, 40},{ 45, 50, 55}},
        {{ 60, 65, 70},{ 75, 80, 85},{ 90, 95,100},{105,110,115}},
        {{120,125,130},{135,140,145},{150,155,160},{165,170,175}}
    };

    vector<vector<Pixel>> result = process_1(tiny);

    for (int row = 0; row < result.size(); row++)
    {
        for (int col = 0; col < result[0].size(); col++)
        {
            cout << setw(3) << result[row][col].red << " ";
            cout << setw(3) << result[row][col].green << " ";
            cout << setw(3) << result[row][col].blue << " ";
        }
        cout << endl;
    }
    return 0;
}

*/
//************************************** Testing block **********************************


