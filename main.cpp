#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

struct RGBPixel {
    int r;
    int g;
    int b;
};

int read_bytes(fstream& file, int position, int byte_count) {
    file.seekg(position);
    int result = 0;
    int multiplier = 1;
    for (int i = 0; i < byte_count; i++) {
        result += file.get() * multiplier;
        multiplier *= 256;
    }
    return result;
}

vector<vector<RGBPixel>> load_bmp(string filename) {
    fstream file;
    string path = "../" + filename;
    file.open(path, ios::in | ios::binary);

    if (!file.is_open()) {
        cout << "Chyba pri otváraní súboru: " << filename << endl;
        return {};
    }

    int total_size = read_bytes(file, 2, 4);
    int data_offset = read_bytes(file, 10, 4);
    int image_width = read_bytes(file, 18, 4);
    int image_height = read_bytes(file, 22, 4);
    int bits_per_pixel = read_bytes(file, 28, 2);

    int row_bytes = image_width * (bits_per_pixel / 8);
    int row_padding = (4 - (row_bytes % 4)) % 4;

    if (total_size != data_offset + (row_bytes + row_padding) * image_height) {
        return {};
    }

    vector<vector<RGBPixel>> pixels(image_height, vector<RGBPixel>(image_width));
    int current_pos = data_offset;

    for (int y = image_height - 1; y >= 0; y--) {
        for (int x = 0; x < image_width; x++) {
            file.seekg(current_pos);
            pixels[y][x].b = file.get();
            pixels[y][x].g = file.get();
            pixels[y][x].r = file.get();
            current_pos += (bits_per_pixel / 8);
        }
        file.seekg(row_padding, ios::cur);
        current_pos += row_padding;
    }

    file.close();
    return pixels;
}

void write_bytes(unsigned char* buffer, int offset, int count, int value) {
    for (int i = 0; i < count; i++) {
        buffer[offset + i] = (unsigned char)(value >> (i * 8));
    }
}

bool save_bmp(string filename, const vector<vector<RGBPixel>>& pixels) {
    string path = "../" + filename;
    int width = pixels[0].size();
    int height = pixels.size();
    int row_bytes = width * 3;
    int padding = (4 - (row_bytes % 4)) % 4;
    int total_row_bytes = row_bytes + padding;
    int image_data_size = total_row_bytes * height;

    fstream file;
    file.open(path, ios::out | ios::binary);

    if (!file.is_open()) {
        cout << "z technickych pricin nemozeme vytvorit novy subor velmi nas to mrzi '" << path << endl;
        return false;
    }

    const int HEADER_SIZE = 14;
    const int INFO_SIZE = 40;
    unsigned char header[HEADER_SIZE] = {0};
    unsigned char info[INFO_SIZE] = {0};

    write_bytes(header, 0, 1, 'B');
    write_bytes(header, 1, 1, 'M');
    write_bytes(header, 2, 4, HEADER_SIZE + INFO_SIZE + image_data_size);
    write_bytes(header, 10, 4, HEADER_SIZE + INFO_SIZE);

    write_bytes(info, 0, 4, INFO_SIZE);
    write_bytes(info, 4, 4, width);
    write_bytes(info, 8, 4, height);
    write_bytes(info, 12, 2, 1);
    write_bytes(info, 14, 2, 24);
    write_bytes(info, 20, 4, image_data_size);
    write_bytes(info, 24, 4, 2835);
    write_bytes(info, 28, 4, 2835);

    file.write((char*)header, HEADER_SIZE);
    file.write((char*)info, INFO_SIZE);

    unsigned char pixel_data[3] = {0};
    unsigned char padding_data[3] = {0};

    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            pixel_data[0] = pixels[y][x].b;
            pixel_data[1] = pixels[y][x].g;
            pixel_data[2] = pixels[y][x].r;
            file.write((char*)pixel_data, 3);
        }
        file.write((char*)padding_data, padding);
    }

    file.close();
    cout << "neboj sa, obrazok si najdes tu : " << path << endl;
    return true;
}

vector<vector<RGBPixel>> make_grayscale(const vector<vector<RGBPixel>>& original) {
    int width = original[0].size();
    int height = original.size();
    vector<vector<RGBPixel>> result(height, vector<RGBPixel>(width));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int gray = (original[y][x].r + original[y][x].g + original[y][x].b) / 3;
            result[y][x].r = gray;
            result[y][x].g = gray;
            result[y][x].b = gray;
        }
    }

    return result;
}

int main() {
    string input_file, output_file;
    cout << "cely nazov suboru aj s \".bmp\": ";
    cin >> input_file;
    cout << "nazov aky chces aby mal prefarbeny subor aj s \".bmp\": ";
    cin >> output_file;

    vector<vector<RGBPixel>> image = load_bmp(input_file);

    if (image.empty()) {
        cout << "Nepodarilo sa načítať vstupný súbor" << endl;
        return 1;
    }

    vector<vector<RGBPixel>> grayscale = make_grayscale(image);
    if (save_bmp(output_file, grayscale)) {
        cout << "mas to tam!" << endl;
    } else {
        cout << "nemas to tam, nieco sa pokazilo (moj kod to nebude)" << endl;
        return 1;
    }

    return 0;
}
