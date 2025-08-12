#include "utils.hh"

// Creates work directory if it doesn't exist
bool ensureWorkDir() {
  struct stat st = {0};
  if (stat(WORK_DIR.c_str(), &st) == -1) {
    // Directory doesn't exist, create it
    return mkdir(WORK_DIR.c_str(), 0755) == 0;
  }
  return true;
}

// Returns full path for a file in the work directory
std::string getWorkPath(const std::string &filename) {
  return WORK_DIR + "/" + filename;
}

// Encodes binary data to base64 string
std::string base64_encode(const std::vector<unsigned char> &data) {
  const std::string chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string result;
  int val = 0, valb = -6;
  for (unsigned char c : data) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      result.push_back(chars[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
  while (result.size() % 4)
    result.push_back('=');
  return result;
}

// Reads a file and returns its content as JSON (text or base64 depending on type)
std::optional<json> encodeFile(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    return std::nullopt;
  }

  // Read file content
  std::vector<unsigned char> buffer((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
  file.close();

  if (buffer.empty()) {
    return std::nullopt;
  }

  // Encode to base64
  std::string encoded = base64_encode(buffer);

  // Determine MIME type based on file extension
  std::string mimeType = "application/octet-stream";
  size_t dotPos = filepath.find_last_of('.');
  if (dotPos != std::string::npos) {
    std::string ext = filepath.substr(dotPos + 1);
    // Images
    if (ext == "png")
      mimeType = "image/png";
    else if (ext == "jpg" || ext == "jpeg")
      mimeType = "image/jpeg";
    else if (ext == "gif")
      mimeType = "image/gif";
    else if (ext == "svg")
      mimeType = "image/svg+xml";
    // Source code files
    else if (ext == "cpp" || ext == "cxx" || ext == "cc")
      mimeType = "text/x-c++src";
    else if (ext == "c")
      mimeType = "text/x-csrc";
    else if (ext == "h" || ext == "hh" || ext == "hpp")
      mimeType = "text/x-c++hdr";
    else if (ext == "js")
      mimeType = "text/javascript";
    else if (ext == "ts")
      mimeType = "text/typescript";
    else if (ext == "py")
      mimeType = "text/x-python";
    else if (ext == "java")
      mimeType = "text/x-java";
    else if (ext == "rs")
      mimeType = "text/x-rust";
    else if (ext == "go")
      mimeType = "text/x-go";
    else if (ext == "dsp")
      mimeType = "application/x-faust";
    else if (ext == "html")
      mimeType = "text/html";
    else if (ext == "css")
      mimeType = "text/css";
    else if (ext == "xml")
      mimeType = "text/xml";
    else if (ext == "json")
      mimeType = "application/json";
    else if (ext == "md")
      mimeType = "text/markdown";
    // Generic text
    else if (ext == "txt")
      mimeType = "text/plain";
    else if (ext == "pdf")
      mimeType = "application/pdf";
  }

  // Create JSON response - use text for text files, base64 for binary
  json result = {{"mimeType", mimeType}};

  if (mimeType.substr(0, 5) == "text/" || mimeType == "application/json" ||
      mimeType == "image/svg+xml" || mimeType == "application/x-faust") {
    // For text files, return as plain text
    std::string textContent(buffer.begin(), buffer.end());
    result["text"] = textContent;
  } else {
    // For binary files, return as base64
    result["data"] = encoded;
  }

  return result;
}
