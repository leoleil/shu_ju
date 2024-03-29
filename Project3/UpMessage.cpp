#include "UpMessage.h"



UpMessage::UpMessage(UINT16 messageId, long long dateTime, bool encrypt, UINT32 taskNum, char * fileName, char * expandName, bool endFlag):Message(messageId, dateTime, encrypt), taskNum(taskNum), endFlag(endFlag)
{
	strcpy_s(this->fileName, 32, fileName);
	strcpy_s(this->expandName, 8, expandName);
	this->messageLength = this->messageLength + sizeof(UINT32) + sizeof(bool) + 32 + 8;
}
UpMessage::UpMessage() {

}

UpMessage::~UpMessage()
{
	delete data;
}


UINT32 UpMessage::getterTaskNum()
{
	return this->taskNum;
}

void UpMessage::getterFileName(char * fileName)
{
	memcpy(fileName, this->fileName, 32);

}

void UpMessage::getterExpandName(char * expandName)
{
	memcpy(expandName, this->expandName, 8);
}

bool UpMessage::getterEndFlag()
{
	return this->endFlag;
}

int UpMessage::getterDataSize()
{
	return size;
}

void UpMessage::setterDataSize(int size)
{
	this->size = size;
}

void UpMessage::getterData(char * data, int & size)
{
	memcpy(data, this->data, this->size);
	size = this->size;
}

void UpMessage::setterData(char * data, int size)
{
	this->data = new char[size];
	memcpy(this->data, data, size);
	this->messageLength = this->messageLength + size;//报文长度加
	this->size = size;//数据大小改变
}

void UpMessage::createMessage(char * buf, int & message_size, int buf_size)
{
	if (buf_size >= messageLength) {
		char* bufPtr = buf;//buf指针
		memcpy(bufPtr, &(messageId), sizeof(UINT16));//任务标志
		bufPtr = bufPtr + sizeof(UINT16);//移动指针
		memcpy(bufPtr, &messageLength, sizeof(UINT32));//报文长度
		bufPtr = bufPtr + sizeof(UINT32);//移动指针
		memcpy(bufPtr, &dateTime, sizeof(long long));//时间戳
		bufPtr = bufPtr + sizeof(long long);//移动指针
		memcpy(bufPtr, &encrypt, sizeof(bool));//加密标识
		bufPtr = bufPtr + sizeof(bool);//移动指针
		memcpy(bufPtr, &taskNum, sizeof(UINT32));//任务编号
		bufPtr = bufPtr + sizeof(UINT32);//移动指针
		memcpy(bufPtr, fileName, 32);//文件名
		bufPtr = bufPtr + 32;//移动指针
		memcpy(bufPtr, expandName, 8);//文件名
		bufPtr = bufPtr + 8;//移动指针
		memcpy(bufPtr, &endFlag, sizeof(bool));//结束标志
		bufPtr = bufPtr + sizeof(bool);//移动指针
		memcpy(bufPtr, data, size);//数据包
		message_size = messageLength;
	}
}

void UpMessage::messageParse(char * buf)
{
	char* bufPtr = buf;//buf指针
	memcpy(&(messageId), bufPtr, sizeof(UINT16));//任务标志
	bufPtr = bufPtr + sizeof(UINT16);//移动指针
	memcpy(&messageLength, bufPtr, sizeof(UINT32));//报文长度
	bufPtr = bufPtr + sizeof(UINT32);//移动指针
	memcpy(&dateTime, bufPtr, sizeof(long long));//时间戳
	bufPtr = bufPtr + sizeof(long long);//移动指针
	memcpy(&encrypt, bufPtr, sizeof(bool));//加密标识
	bufPtr = bufPtr + sizeof(bool);//移动指针
	memcpy(&taskNum, bufPtr, sizeof(UINT32));//任务编号
	bufPtr = bufPtr + sizeof(UINT32);//移动指针
	memcpy(fileName, bufPtr, 32);//文件名
	bufPtr = bufPtr + 32;//移动指针
	memcpy(expandName, bufPtr, 8);//文件名
	bufPtr = bufPtr + 8;//移动指针
	memcpy(&endFlag, bufPtr, sizeof(bool));//结束标志
	bufPtr = bufPtr + sizeof(bool);//移动指针
	size = messageLength - (bufPtr - buf);
	this->data = new char[size];
	memcpy(data, bufPtr, size);//数据包
}
