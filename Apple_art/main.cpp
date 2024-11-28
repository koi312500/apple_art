#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace cv;
using namespace std;

vector<int> apple_sizes = { 256, 128, 64, 32, 16, 8, 4, 2, 1 };
size_t current_apple_size_index = 0;
size_t current_image_index = 0;
vector<string> image_files;
Mat canvas(1440, 960, CV_8UC3, Scalar(255, 255, 255));
Mat apple_original, apple_pixel;

void process_image();
void mouse_callback(int event, int x, int y, int flags, void* userdata);

int main() {
    apple_original = imread("apple.png", IMREAD_COLOR);
    if (apple_original.empty()) {
        cout << "Error loading the apple image!" << endl;
        return -1;
    }

    string folderPath = "resources";
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        string filePath = entry.path().string();
        if (entry.is_regular_file() &&
            (filePath.ends_with(".png") || filePath.ends_with(".jpg") || filePath.ends_with(".jpeg"))) {
            image_files.push_back(filePath);
        }
    }

    if (image_files.empty()) {
        cout << "No valid images found in folder: " << folderPath << endl;
        return -1;
    }

    process_image();

    namedWindow("Canvas with Gradient Apples");
    setMouseCallback("Canvas with Gradient Apples", mouse_callback);

    imshow("Canvas with Gradient Apples", canvas);
    waitKey(0);
    destroyAllWindows();
    return 0;
}

void process_image() {
    if (current_image_index >= image_files.size()) {
        cout << "All images processed." << endl;
        return;
    }

    string filePath = image_files[current_image_index];
    Mat print_image = imread(filePath, IMREAD_GRAYSCALE);
    if (print_image.empty()) {
        cout << "Failed to load image: " << filePath << endl;
        current_image_index++;
        process_image();
        return;
    }

    cout << "Processing image: " << filePath << endl;
    resize(print_image, print_image, canvas.size());
    canvas.setTo(Scalar(255, 255, 255));

    int apple_size = apple_sizes[current_apple_size_index];
    resize(apple_original, apple_pixel, Size(apple_size, apple_size));

    for (int y = 0; y < print_image.rows; y += apple_size) {
        for (int x = 0; x < print_image.cols; x += apple_size) {
            uchar gray_value = print_image.at<uchar>(y, x);

            if (gray_value == 0) continue;

            double alpha = gray_value / 255.0;
            Mat apple_tinted = apple_pixel.clone();

            for (int ay = 0; ay < apple_tinted.rows; ay++) {
                for (int ax = 0; ax < apple_tinted.cols; ax++) {
                    Vec3b& pixel = apple_tinted.at<Vec3b>(ay, ax);
                    if (!(pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0)) {
                        pixel[1] = static_cast<uchar>((1 - alpha) * 255);
                        pixel[2] = static_cast<uchar>(alpha * 255);
                    }
                }
            }

            Rect roi(x, y, apple_size, apple_size);
            if (roi.x + roi.width <= canvas.cols && roi.y + roi.height <= canvas.rows) {
                apple_tinted.copyTo(canvas(roi));
            }
        }
    }
}

void mouse_callback(int event, int x, int y, int flags, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        current_apple_size_index++;
        if (current_apple_size_index >= apple_sizes.size()) {
            current_apple_size_index = 0;
            current_image_index++;
        }

        if (current_image_index < image_files.size()) {
            process_image();
            imshow("Canvas with Gradient Apples", canvas);
        }
        else {
            cout << "All images processed. Closing..." << endl;
            destroyWindow("Canvas with Gradient Apples");
        }
    }
}
