#include <iostream>
#include <fstream>
#include <vector>

 using namespace std;

 struct Pixely{
     int cervena;
     int zelena;
     int modra;
 };
 
 int get_int(fstream& stream, int offset, int bytes){
     stream.seekg(offset);
     int vysledok = 0;
     int base = 1;
     for (int i = 0; i < bytes; i++){   
         vysledok = vysledok + stream.get() * base;
         base = base * 256;
     }
     return vysledok;
 }
 
 vector<vector<Pixely>> read_image(string filename){
     
     fstream stream;
     string fullPath = "../" + filename;
     stream.open(fullPath, ios::in | ios::binary);
 
     if (!stream.is_open()) {
         cout << "subor sa neotvoril '" << filename << "'" << endl;
         return {};
     }
 
     int file_size = get_int(stream, 2, 4);
     //cout << "file_size: " << file_size << endl;
     int start = get_int(stream, 10, 4);
     int width = get_int(stream, 18, 4);
     int height = get_int(stream, 22, 4);
     int bites_per_pixel = get_int(stream, 28, 2);
 
    int bytes_per_row = width * (bites_per_pixel / 8);
     int padding = 0;
     if (bytes_per_row % 4 != 0){
         padding = 4 - bytes_per_row % 4;
     }
 
     if (file_size != start + (bytes_per_row + padding) * height){
         return {};
     }
 
     vector<vector<Pixely> > image(height, vector<Pixely> (width));
     int pos = start;
     for (int i = height - 1; i >= 0; i--){
       
         for (int j = 0; j < width; j++){
             stream.seekg(pos);
 
             image[i][j].modra = stream.get();
             image[i][j].zelena = stream.get();
             image[i][j].cervena = stream.get();
 
             pos = pos + (bites_per_pixel / 8);
         }
         stream.seekg(padding, ios::cur);
         pos = pos + padding;
     }
 
   	stream.close();
     return image;
 }
 
 void set_bytes(unsigned char arr[], int offset, int bytes, int value){
     for (int i = 0; i < bytes; i++){
 	arr[offset+i] = (unsigned char)(value>>(i*8));
     }
 }
 
 bool write_image(string filename, const vector<vector<Pixely>>& image) {
 
     string fullPath = "../" + filename;
     int width_pixels = image[0].size();
     int height_pixels = image.size();
     int bytes_per_row = width_pixels * 3;
     int padding_bytes = (4 - bytes_per_row % 4) % 4;
     bytes_per_row += padding_bytes;
     int array_bytes = bytes_per_row * height_pixels;
 
     fstream stream;
     stream.open(fullPath, ios::out | ios::binary);
 
     if (!stream.is_open()) {
         cout << "z technickych pricin nemozeme vytvorit novy subor velmi nas to mrzi '" << fullPath << "'" << endl;
         return false;
     }
 
     const int BMP_HEADER_SIZE = 14;
     const int DIB_HEADER_SIZE = 40;
     unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
     unsigned char dib_header[DIB_HEADER_SIZE] = {0};
 
     set_bytes(bmp_header,  0, 1, 'B');
     set_bytes(bmp_header,  1, 1, 'M');
     set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE + array_bytes);
     set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE);
 
     set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);
     set_bytes(dib_header,  4, 4, width_pixels);
     set_bytes(dib_header,  8, 4, height_pixels);
     set_bytes(dib_header, 12, 2, 1);
     set_bytes(dib_header, 14, 2, 24);
     set_bytes(dib_header, 20, 4, array_bytes);
     set_bytes(dib_header, 24, 4, 2835);
     set_bytes(dib_header, 28, 4, 2835);
 
     stream.write((char*)bmp_header, sizeof(bmp_header));  
     stream.write((char*)dib_header, sizeof(dib_header));
 
     unsigned char pixel[3] = {0};
     unsigned char padding[3] = {0};
 
     for (int h = height_pixels - 1; h >= 0; h--) {
         for (int w = 0; w < width_pixels; w++) {
             pixel[0] = image[h][w].modra;
             pixel[1] = image[h][w].zelena;
             pixel[2] = image[h][w].cervena;
             stream.write((char*)pixel, 3);
         }
         stream.write((char*)padding, padding_bytes);
     }
 
     stream.close();
     cout << "neboj sa, obrazok si najdes tu : " << fullPath << endl;
     return true;
 }
 vector<vector<Pixely> > convert_to_grayscale(const vector<vector<Pixely>>& image) {
     
     int width_pixels = image[0].size();
     int height_pixels = image.size();
 
      vector<vector<Pixely> > new_image(height_pixels, vector<Pixely> (width_pixels));
 
     for (int row = 0; row < height_pixels; row++) { 
         for (int col = 0; col < width_pixels; col++) {
 
             int red = image[row][col].cervena;
             int green = image[row][col].zelena;
             int blue = image[row][col].modra;
 
            	int gray_value = ((red + green + blue)/3) ;
 
       		new_image[row][col].cervena = gray_value;
             new_image[row][col].zelena = gray_value;
             new_image[row][col].modra = gray_value;
         }
     }
 
   return new_image;
 }
 
 int main(){
     string inputFilename, outputFilename;
     cout << "cely nazov suboru aj s \".bmp\": ";
     cin >> inputFilename;
     cout << "nazov aky chces aby mal prefarbeny subor aj s \".bmp\": ";
     cin >> outputFilename;
     
     vector<vector<Pixely> > image = read_image(inputFilename);
     
     if (image.empty()) {
         cout << "subor sa neotvoril " << endl;
         return 1;
     }
     
 	vector<vector<Pixely> > grayscaleImage = convert_to_grayscale(image);
     if (write_image(outputFilename, grayscaleImage)) {
         cout << "mas to tam!" << endl;
     } else {
         cout << "nemas to tam, nieco sa pokazilo (moj kod to nebude)" << endl;
         return 1;
     }
     return 0;
 }