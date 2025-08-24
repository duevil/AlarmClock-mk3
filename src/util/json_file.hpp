#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <ArduinoJson.h>
#include <SD.h>


/**
 * Utility class extending a JSON document for reading and writing a JSON file using RAII
 */
class JsonFile : public JsonDocument
{
    static constexpr auto op_read = 0b01; // Read operation
    static constexpr auto op_write = 0b10; // Write operation

    const char* path_;
    FS& fs_;
    uint8_t op_;
    DeserializationError error_{};

    JsonFile(const char* path, FS& fs, uint8_t op): path_(path), fs_(fs), op_(op)
    {
        // perform reading only if read operation was requested
        if (op_ & op_read)
        {
            if (fs_.exists(path_))
            {
                if (auto file = fs_.open(path_, FILE_READ))
                {
                    error_ = deserializeJson(*this, file);
                    file.close();
                }
            }
            else
            {
                error_ = DeserializationError::EmptyInput;
            }
        }
    }

public:
    /**
     * Creates an instance reading a JSON file; changes will not be written
     * @param path The path of the file
     * @param fs The filesystem the file should be read from
     * @return The JsonFile instance
     */
    static JsonFile r(const char* path, FS& fs = SD) { return {path, fs, op_read}; }
    /**
     * Creates an instance writing a JSON file; contents will not be read (the document will be emtpy)
     * @param path The path of the file
     * @param fs The filesystem the file should be written to
     * @return The JsonFile instance
     */
    static JsonFile w(const char* path, FS& fs = SD) { return {path, fs, op_write}; }
    /**
     * Creates an instance both reading and writing a JSON file
     * @param path The path of the file
     * @param fs The filesystem the file should be read from and written to
     * @return The JsonFile instance
     */
    static JsonFile rw(const char* path, FS& fs = SD) { return {path, fs, op_read | op_write}; }

    // use destructor to automatically write the JSON document
    // if requested after the instance gets out of scope
    ~JsonFile()
    {
        // perform writing only if write operation was requested
        if (op_ & op_write)
        {
            // on ESP32, FILE_WRITE will overwrite existing contents
            auto file = fs_.open(path_, FILE_WRITE);
            if (file)
            {
                serializeJsonPretty(*this, file);
            }
            file.close();
        }
    }

    /**
     * Get the possible deserialization error that might have occurred during reading the JSON file
     * @return The deserialization error (DeserializationError::OK if no error occurred or no read was performed)
     */
    [[nodiscard]] auto error() const { return error_; }


    // disallow copying and moving to prevent multiple destructor calls

    JsonFile(JsonFile&) = delete;
    JsonFile(JsonFile&&) = delete;
};


#endif //CONFIG_FILE_HPP
