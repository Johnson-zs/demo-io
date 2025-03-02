#pragma once
#include "slave.h"

class ImageProcessor : public TaskExecutor {
    Q_OBJECT
public:
    explicit ImageProcessor(const TaskMessage &task, QObject *parent = nullptr);
    
    void start() override;
    void stop() override;
    
private:
    void processImage();
}; 