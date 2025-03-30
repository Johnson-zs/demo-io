#include "framework/message.h"
#include <QDebug>
#include <QUuid>

namespace Framework {

// Message 实现
Message::Message() : timestamp(QDateTime::currentDateTime()) {
    messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

Message::Message(MessageType type) : type(type), timestamp(QDateTime::currentDateTime()) {
    messageId = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Message::serialize(QDataStream &out) const {
    out << static_cast<int>(type) << messageId << timestamp;
}

void Message::deserialize(QDataStream &in) {
    int typeInt;
    in >> typeInt >> messageId >> timestamp;
    type = static_cast<MessageType>(typeInt);
}

QDataStream &operator<<(QDataStream &out, const Message &msg) {
    msg.serialize(out);
    return out;
}

QDataStream &operator>>(QDataStream &in, Message &msg) {
    msg.deserialize(in);
    return in;
}

// RegisterMessage 实现
RegisterMessage::RegisterMessage() : Message(MessageType::REGISTER) {
}

void RegisterMessage::serialize(QDataStream &out) const {
    Message::serialize(out);
    out << workerId << hostname << cores << totalMemory << capabilities;
}

void RegisterMessage::deserialize(QDataStream &in) {
    Message::deserialize(in);
    in >> workerId >> hostname >> cores >> totalMemory >> capabilities;
}

// HeartbeatMessage 实现
HeartbeatMessage::HeartbeatMessage() : Message(MessageType::HEARTBEAT) {
}

void HeartbeatMessage::serialize(QDataStream &out) const {
    Message::serialize(out);
    out << workerId << cpuUsage << memoryUsage << runningTaskIds;
}

void HeartbeatMessage::deserialize(QDataStream &in) {
    Message::deserialize(in);
    in >> workerId >> cpuUsage >> memoryUsage >> runningTaskIds;
}

// TaskMessage 实现
TaskMessage::TaskMessage() : Message(MessageType::TASK_ASSIGN), priority(0) {
}

void TaskMessage::serialize(QDataStream &out) const {
    Message::serialize(out);
    out << taskId << workerId << taskType << parameters << priority << deadline;
}

void TaskMessage::deserialize(QDataStream &in) {
    Message::deserialize(in);
    in >> taskId >> workerId >> taskType >> parameters >> priority >> deadline;
}

// TaskStatusMessage 实现
TaskStatusMessage::TaskStatusMessage() : Message(MessageType::TASK_STATUS), progress(0.0) {
}

void TaskStatusMessage::serialize(QDataStream &out) const {
    Message::serialize(out);
    out << taskId << workerId << static_cast<int>(status) << progress << statusMessage;
}

void TaskStatusMessage::deserialize(QDataStream &in) {
    Message::deserialize(in);
    int statusInt;
    in >> taskId >> workerId >> statusInt >> progress >> statusMessage;
    status = static_cast<Status>(statusInt);
}

// TaskResultMessage 实现
TaskResultMessage::TaskResultMessage() : Message(MessageType::TASK_RESULT), success(false) {
}

void TaskResultMessage::serialize(QDataStream &out) const {
    Message::serialize(out);
    out << taskId << workerId << success << result;
}

void TaskResultMessage::deserialize(QDataStream &in) {
    Message::deserialize(in);
    in >> taskId >> workerId >> success >> result;
}

} // namespace Framework 