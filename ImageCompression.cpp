#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cstdio>
#include <vector>
#include <bitset>
#include <unordered_map>
#include <queue>
#include <functional>

using namespace cv;

// Function to get the file size
long getFileSize(const std::string& filePath) {
    FILE* file = nullptr;
    errno_t err = fopen_s(&file, filePath.c_str(), "rb");
    if (err != 0 || file == nullptr) {
        std::cerr << "Error: Could not open the file!" << std::endl;
        return -1;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

// Function to resize image to fit within a specified window size
Mat resizeToFit(const Mat& img, int maxWidth, int maxHeight) {
    int originalWidth = img.cols;
    int originalHeight = img.rows;

    if (originalWidth <= maxWidth && originalHeight <= maxHeight) {
        return img;
    }

    float aspectRatio = static_cast<float>(originalWidth) / static_cast<float>(originalHeight);
    int newWidth, newHeight;

    if (originalWidth > originalHeight) {
        newWidth = maxWidth;
        newHeight = static_cast<int>(maxWidth / aspectRatio);
    }
    else {
        newHeight = maxHeight;
        newWidth = static_cast<int>(maxHeight * aspectRatio);
    }

    Mat resizedImg;
    cv::resize(img, resizedImg, Size(newWidth, newHeight));
    return resizedImg;
}

// Function to perform Run-Length Encoding (RLE)
std::vector<uchar> runLengthEncode(const std::vector<uchar>& data) {
    std::vector<uchar> encodedData;
    for (size_t i = 0; i < data.size(); ++i) {
        uchar count = 1;
        while (i + 1 < data.size() && data[i] == data[i + 1]) {
            ++count;
            ++i;
        }
        encodedData.push_back(data[i]);
        encodedData.push_back(count);
    }
    return encodedData;
}

// Function to build Huffman Tree
struct HuffmanNode {
    uchar data;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;
    HuffmanNode(uchar d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

struct HuffmanNodeCompare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return (l->freq > r->freq);
    }
};

void buildHuffmanTree(const std::vector<uchar>& data, std::unordered_map<uchar, std::string>& huffmanCode) {
    std::unordered_map<uchar, int> freq;
    for (uchar ch : data) {
        freq[ch]++;
    }

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, HuffmanNodeCompare> pq;
    for (auto pair : freq) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    while (pq.size() != 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        int sum = left->freq + right->freq;
        HuffmanNode* node = new HuffmanNode('\0', sum);
        node->left = left;
        node->right = right;
        pq.push(node);
    }

    HuffmanNode* root = pq.top();

    std::function<void(HuffmanNode*, std::string)> encode = [&](HuffmanNode* node, std::string str) {
        if (node == nullptr) return;
        if (!node->left && !node->right) {
            huffmanCode[node->data] = str;
        }
        encode(node->left, str + "0");
        encode(node->right, str + "1");
        };

    encode(root, "");
}

// Function to perform Huffman Encoding
std::vector<uchar> huffmanEncode(const std::vector<uchar>& data) {
    std::unordered_map<uchar, std::string> huffmanCode;
    buildHuffmanTree(data, huffmanCode);

    std::string encodedStr;
    for (uchar ch : data) {
        encodedStr += huffmanCode[ch];
    }

    std::vector<uchar> encodedData;
    for (size_t i = 0; i < encodedStr.size(); i += 8) {
        std::bitset<8> bits(encodedStr.substr(i, 8));
        encodedData.push_back(static_cast<uchar>(bits.to_ulong()));
    }

    return encodedData;
}

int main() {
    // Path to the original image
    std::string inputPath = "E:/Genshin Impact game/ScreenShot/20240322123406.png";
    // Path to save the compressed image
    std::string compressedOutputPath = "D:/Compress/compressed_tohru.jpg";
    // Path to save the losslessly compressed image
    std::string losslessOutputPath = "D:/Compress/lossless_tohru.png";

    // Read the image
    Mat img = imread(inputPath);

    // Check if the image was loaded successfully
    if (img.empty()) {
        std::cerr << "Error: Could not open or find the image!" << std::endl;
        return -1;
    }

    // Get the size of the new image file
    long originalFileSize = getFileSize(inputPath);
    if (originalFileSize != -1) {
        std::cout << "Size of the original image: " << originalFileSize << " bytes" << std::endl;
    }

    // Compress and save the image with 50% quality
    std::vector<int> compressionParams;
    compressionParams.push_back(IMWRITE_JPEG_QUALITY);
    compressionParams.push_back(50); // 50% quality

    imwrite(compressedOutputPath, img, compressionParams);

    // Get the size of the compressed image file
    long compressedFileSize = getFileSize(compressedOutputPath);
    if (compressedFileSize != -1) {
        std::cout << "Size of the compressed image: " << compressedFileSize << " bytes" << std::endl;
    }

    // Save the image using PNG format for lossless compression
    imwrite(losslessOutputPath, img);

    // Get the size of the losslessly compressed image file
    long losslessFileSize = getFileSize(losslessOutputPath);
    if (losslessFileSize != -1) {
        std::cout << "Size of the losslessly compressed image: " << losslessFileSize << " bytes" << std::endl;
    }

    // Read and display the compressed image
    Mat compressedImg = imread(compressedOutputPath);

    // Resize the image to fit within the specified window size
    int maxWidth = 800; // Maximum window width
    int maxHeight = 600; // Maximum window height
    Mat resizedImg = resizeToFit(compressedImg, maxWidth, maxHeight);

    namedWindow("Compressed Image", WINDOW_AUTOSIZE);
    imshow("Compressed Image", resizedImg);
    moveWindow("Compressed Image", 0, 0);

    // Wait for a key press and close the windows
    waitKey(0);
    destroyAllWindows();

    return 0;
}
