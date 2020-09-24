#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "wchar.h"
#include "locale.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

#define MAX_LENGTH_OF_COMMAND 128
#define NAME_LENGTH 32
#define SURNAME_LENGTH 32
#define PHONE_LENGTH 16
#define CITY_LENGTH 16

typedef struct Delivery {
	int id;
	char city[CITY_LENGTH];
	int departmentNumber;
	int nextDeliveryID;
	int senderID;
	bool isDeleted;
} Delivery;

typedef struct Sender {
	int id;
	char name[NAME_LENGTH];
	char surname[SURNAME_LENGTH];
	char phone[PHONE_LENGTH];
	int firstDeliveryID;
	bool isDeleted;
} Sender;

typedef struct FileHandler {
	FILE* indexFile;
	FILE* deliveryFile;
	FILE* senderFile;
	char* indexFileName;
	char* deliveryFileName;
	char* senderFileName;
}FileHandler;

FileHandler handler;
char delims[] = " \n";

//helper
void insertToIndexFile(Sender* newSender) {
	handler.indexFile = fopen(handler.indexFileName, "rb+");
	int oldIndex;
	fread(&oldIndex, sizeof(int), 1, handler.indexFile);
	fseek(handler.indexFile, 0L, SEEK_SET);
	int newIndex = oldIndex + 1;
	fwrite(&newIndex, sizeof(int), 1, handler.indexFile);
	fseek(handler.indexFile, 0L, SEEK_END);
	fwrite(&(newSender->id), sizeof(int), 1, handler.indexFile);
	fwrite(&oldIndex, sizeof(int), 1, handler.indexFile);
	fclose(handler.indexFile);
}

//main function
void insertToMasterFile(char command[MAX_LENGTH_OF_COMMAND]) {

	strtok(command, delims);
	int id = atoi(strtok(NULL, delims));


	if (getAddress(id) != -1) {
		printf("Sender with such id already exists\n");
		return;
	}

	Sender newSender;
	newSender.id = id;
	strncpy(newSender.name, strtok(NULL, delims), NAME_LENGTH);
	strncpy(newSender.surname, strtok(NULL, delims), SURNAME_LENGTH);
	strncpy(newSender.phone, strtok(NULL, delims), PHONE_LENGTH);
	newSender.firstDeliveryID = -1;
	newSender.isDeleted = false;
	insertToIndexFile(&newSender);

	handler.senderFile = fopen(handler.senderFileName, "ab+");
	fwrite(&newSender, sizeof(Sender), 1, handler.senderFile);;
	fclose(handler.senderFile);
}

//helper
int getAddress(int id) {
	int address;

	handler.indexFile = fopen(handler.indexFileName, "rb");
	fseek(handler.indexFile, sizeof(int), SEEK_SET);

	int tempNumber;
	bool found = false;
	while (fread(&tempNumber, sizeof(int), 1, handler.indexFile)) {
		if (tempNumber == id) {
			fread(&tempNumber, sizeof(int), 1, handler.indexFile);
			found = true;
			break;
		}
		fseek(handler.indexFile, sizeof(int), SEEK_CUR);
	}
	fclose(handler.indexFile);

	if (found)
		return tempNumber;

	return -1;
}

//helper (DONT CALL IF address < 0)
Sender getByAddress(int address) {
	if (address == -1) {
		printf("invalid address");
		return;
	}

	handler.senderFile = fopen(handler.senderFileName, "rb");
	fseek(handler.senderFile, (long)address * sizeof(Sender), SEEK_SET);

	Sender tempSender;
	fread(&tempSender, sizeof(Sender), 1, handler.senderFile);
	fclose(handler.senderFile);
	return tempSender;
}

//helper
void printMasterRecord(int address) {
	if (address == -1) {
		printf("%s", "The sender with such ID doesnot exist\n");
		return;
	}

	Sender sender = getByAddress(address);

	if (!sender.isDeleted) {
		printf("Sender: %s %s, passport ID #%d, phone number %s\n", \
			sender.name, sender.surname, sender.id, sender.phone);
	}
	else {
		printf("Such sender was deleted");
	}
}

//main function
void getRecordFromMaster(char* command) {
	strtok(command, delims);
	int id = atoi(strtok(NULL, delims));
	int address = getAddress(id);

	printMasterRecord(address);
}

//helper
Delivery getDelivery(int id) {
	handler.deliveryFile = fopen(handler.deliveryFileName, "rb");

	Delivery delivery;
	bool found = false;

	while (fread(&delivery, sizeof(Delivery), 1, handler.deliveryFile)) {
		if (delivery.id == id && !delivery.isDeleted) {
			found = true;
			break;
		}
	}

	fclose(handler.deliveryFile);

	if (!found) {
		delivery.id = -1;
	}

	return delivery;
}

//main function
void printMaster() {
	handler.senderFile = fopen(handler.senderFileName, "rb+");
	Sender tempSender;
	while (fread(&tempSender, sizeof(Sender), 1, handler.senderFile)) {
		//if (!tempSender.isDeleted) {
			printf("Sender: %s %s, passport ID #%d, phone number %s, first delivery #%d, deleted:%d\n", \
				tempSender.name, tempSender.surname, tempSender.id, tempSender.phone, tempSender.firstDeliveryID, tempSender.isDeleted);
		//}
	}
}

//main function
void printSlave() {
	handler.deliveryFile = fopen(handler.deliveryFileName, "rb+");

	Delivery tempDelivery;
	while (fread(&tempDelivery, sizeof(Delivery), 1, handler.deliveryFile)) {
		//if (!tempDelivery.isDeleted) {
			printf("Delivery #%d to the %s, department #%d, next delivery record #%d,sent by passport #%d deleted:%d\n", \
				tempDelivery.id, tempDelivery.city, tempDelivery.departmentNumber, tempDelivery.nextDeliveryID, tempDelivery.senderID, tempDelivery.isDeleted);
		//}
	}
}

//helper
void printSlaveRecord(int id) {
	handler.deliveryFile = fopen(handler.deliveryFileName, "rb+");
	Delivery tempDelivery;
	bool found = false;
	while (fread(&tempDelivery, sizeof(Delivery), 1, handler.deliveryFile)) {
		if (tempDelivery.id == id && !tempDelivery.isDeleted) {
			found = true;
			printf("Delivery to the %s, department #%d\n", \
				tempDelivery.city, tempDelivery.departmentNumber);
		}
	}

	if (!found) {
		printf("Deliveries with such id do not exist\n");
	}

	fclose(handler.deliveryFile);
}

//main function
void getRecordFromSlave(char* command) {
	strtok(command, delims);
	int id = atoi(strtok(NULL, delims));

	printSlaveRecord(id);
}

//helper
void writeToSlaveFile(Delivery* delivery) {
	handler.senderFile = fopen(handler.senderFileName, "ab+");
	fwrite(delivery, sizeof(Delivery), 1, handler.senderFile);
	fclose(handler.senderFile);
}

//helper 
void updateMasterFile(Sender* sender, int address) {
	handler.senderFile = fopen(handler.senderFileName, "rb+");
	fseek(handler.senderFile, (long)address * sizeof(Sender), SEEK_SET);
	fwrite(sender, sizeof(Sender), 1, handler.senderFile);
	fclose(handler.senderFile);
}

//main function
void updateMasterFileWrapper(char command[MAX_LENGTH_OF_COMMAND]) {
	strtok(command, delims);
	int id = atoi(strtok(NULL, delims));
	int address = getAddress(id);

	if (address == -1) {
		printf("Such sender does not exist");
		return;
	}

	Sender sender = getByAddress(address);

	char* name = strtok(NULL, delims);
	if (strcmp(name, "-")) {
		strcpy(sender.name, name);
	}
	char* surName = strtok(NULL, delims);
	if (strcmp(surName, "-")) {
		strcpy(sender.surname, surName);
	}
	char* phone = strtok(NULL, delims);
	if (strcmp(phone, "-")) {
		strcpy(sender.phone, phone);
	}

	updateMasterFile(&sender, address);
}

//helper 
void updateSlaveFile(Delivery* toReplace) {
	handler.deliveryFile = fopen(handler.deliveryFileName, "rb+");
	Delivery tempDelivery;
	int deliveryID = toReplace->id;

	while (fread(&tempDelivery, sizeof(Delivery), 1, handler.deliveryFile)) {
		if (tempDelivery.id == deliveryID && !tempDelivery.isDeleted) {
			fseek(handler.deliveryFile, -1L * sizeof(Delivery), SEEK_CUR);
			fwrite(toReplace, sizeof(Delivery), 1, handler.deliveryFile);
			break;
		}
	}

	fclose(handler.deliveryFile);
}

//main function
void updateSlaveFileWrapper(char command[MAX_LENGTH_OF_COMMAND]) {
	strtok(command, delims);
	int id = atoi(strtok(NULL, delims));

	Delivery delivery = getDelivery(id);
	
	if (delivery.id == -1 || delivery.isDeleted ) {
		printf("Delivery with such ID was deleted or doesnot exist\n");
		return;
	}

	char* city = strtok(NULL, delims);
	if (strcmp(city, "-")) {
		strcpy(delivery.city, city);
	}
	int departmentNumber = atoi(strtok(NULL, delims));
	if (departmentNumber != -1) {
		delivery.departmentNumber = departmentNumber;
	}

	updateSlaveFile(&delivery);
}

//helper
bool slaveRecordPresent(int delivery_id) {
	handler.deliveryFile = fopen(handler.deliveryFileName, "rb");

	Delivery delivery;
	bool found = false;
	while (fread(&delivery, sizeof(Delivery), 1, handler.deliveryFile)) {
		if (delivery.id == delivery_id && !delivery.isDeleted) {
			found = true;
			break;
		}
	}

	fclose(handler.deliveryFile);

	return found;
}

//main function
void insertToSlaveFile(char command[MAX_LENGTH_OF_COMMAND]) {
	strtok(command, delims);
	int senderID = atoi(strtok(NULL, delims));
	int deliveryID = atoi(strtok(NULL, delims));
	char* city = strtok(NULL, delims);
	int departmentNumber = atoi(strtok(NULL, delims));

	Delivery delivery;
	delivery.id = deliveryID;
	strcpy(delivery.city, city);
	delivery.departmentNumber = departmentNumber;
	delivery.nextDeliveryID = -1;
	delivery.senderID = senderID;
	delivery.isDeleted = false;

	int address = getAddress(senderID);

	if (address == -1) {
		printf("Wrong sender ID\n");
		return;
	}

	Sender sender = getByAddress(address);

	if (sender.firstDeliveryID == -1) {
		sender.firstDeliveryID = delivery.id;
	}
	else {
		if (slaveRecordPresent(delivery.id)) {
			printf("Delivery with such ID already exists\n");
			return;
		}

		delivery.nextDeliveryID = sender.firstDeliveryID;
		sender.firstDeliveryID = delivery.id;
	}

	handler.deliveryFile = fopen(handler.deliveryFileName, "ab+");
	fwrite(&delivery, sizeof(Delivery), 1, handler.deliveryFile);
	fclose(handler.deliveryFile);

	updateMasterFile(&sender, address);
}

//helper
void deleteSlaves(int id) {
	handler.deliveryFile = fopen(handler.deliveryFileName, "rb+");

	Delivery delivery;
	int finished = false;
	int address = 0;
	while (fread(&delivery, sizeof(Delivery), 1, handler.deliveryFile)) {
		if (finished) {
			break;
		}
		if (delivery.id == id && !delivery.isDeleted) {
			int next = delivery.nextDeliveryID;

				delivery.isDeleted = true;
				fseek(handler.deliveryFile, -1L * sizeof(Delivery), SEEK_CUR);
				fwrite(&delivery, sizeof(Delivery), 1, handler.deliveryFile);
				if (next == -1) {
					finished = true;
					break;
				}
				id = next;
				fseek(handler.deliveryFile, 0L, SEEK_SET);
		}
	}

	if (!finished) {
		printf("Deletion reached end of file, some of the record might stayed");
	}

	fclose(handler.deliveryFile);
}

//main
void deleteMaster(char command[MAX_LENGTH_OF_COMMAND]) {
	strtok(command, delims);
	int id = atoi(strtok(NULL, delims));

	int address = getAddress(id);

	if (address == -1) {
		printf("Such sender does not exist\n");
		return;
	}

	Sender sender = getByAddress(address);
	
	if (sender.isDeleted) {
		printf("Such sender was deleted\n");
		return;
	}

	sender.isDeleted = true;

	updateMasterFile(&sender,address);

	deleteSlaves(sender.firstDeliveryID);
}

void deleteSlave(char command[MAX_LENGTH_OF_COMMAND]) {
	strtok(command, delims);
	int id = atoi(strtok(NULL, delims));
	Delivery delivery = getDelivery(id);
	if (delivery.id == -1) {
		printf("Delivery with such ID does not exits\n");
		return;
	}

	delivery.isDeleted = true;
	updateSlaveFile(&delivery);

	int senderAddress = getAddress(delivery.senderID);
	
	if (senderAddress == -1) {
		printf("Trying to delete delivery with no owner\n");
	}

	Sender sender = getByAddress(senderAddress);

	if (sender.firstDeliveryID == id) {
		sender.firstDeliveryID = delivery.nextDeliveryID;
		updateMasterFile(&sender, senderAddress);
	}
	else {
		Delivery tempDelivery = getDelivery(sender.firstDeliveryID);
		while (tempDelivery.nextDeliveryID != id) {
			tempDelivery = getDelivery(tempDelivery.nextDeliveryID);
		}

		tempDelivery.nextDeliveryID = delivery.nextDeliveryID;
		updateSlaveFile(&tempDelivery);
	}
}

void runGB() {

}

//helper
void handleCommand() {
	char command[MAX_LENGTH_OF_COMMAND];
	fgets(command, MAX_LENGTH_OF_COMMAND, stdin); //read full line
	char copyOfCommand[MAX_LENGTH_OF_COMMAND];
	memcpy(copyOfCommand, command, MAX_LENGTH_OF_COMMAND);

	char* option = strtok(command, delims);

	if (!strcmp(option, "insert-m")) {
		insertToMasterFile(&copyOfCommand);
	}
	else if (!strcmp(option, "insert-s")) {
		insertToSlaveFile(&copyOfCommand);
	}
	else if (!strcmp(option, "get-m")) {
		getRecordFromMaster(&copyOfCommand);
	}
	else if (!strcmp(option, "get-s")) {
		getRecordFromSlave(&copyOfCommand);
	}
	else if (!strcmp(option, "get-m-all")) {
		printMaster();
	}
	else if (!strcmp(option, "get-s-all")) {
		printSlave();
	}
	else if (!strcmp(option, "del-m")) {
		deleteMaster(&copyOfCommand);
	}
	else if (!strcmp(option, "del-s")) {
		deleteSlave(&copyOfCommand);
	}
	else if (!strcmp(option, "update-m")) {
		updateMasterFileWrapper(&copyOfCommand);
	}
	else if (!strcmp(option, "update-s")) {
		updateSlaveFileWrapper(copyOfCommand);
	}
	else {
		printf("Invalid command\n");
	}
}

void printMenu();
bool fempty(FILE* f);

int main() {
	handler.indexFileName = "sender.ind";
	handler.senderFileName = "sender.fl";
	handler.deliveryFileName = "delivery.fl";

	handler.indexFile = fopen(handler.indexFileName, "ab");
	handler.senderFile = fopen(handler.senderFileName, "ab+");
	handler.deliveryFile = fopen(handler.deliveryFileName, "ab+");
	fclose(handler.senderFile);
	fclose(handler.deliveryFile);

	if (fempty(handler.indexFile)) {
		int firstIndex = 0;
		fwrite(&firstIndex, sizeof(int), 1, handler.indexFile);
	}
	fclose(handler.indexFile);

	printMenu();

	while (true) {
		handleCommand();
	}

	return 0;
}

//helper
bool fempty(FILE* f) {
	fseek(f, 0, SEEK_END);

	unsigned long length = ftell(f);

	if (length != 0)
		return false;

	return true;
}

//helper
void printMenu() {
	printf("This is a delivery database\n");
	printf("Please, enter one of the following commands with parameters\n");
	printf("\t\"insert-m id name surname phone\" - add new sender\n");
	printf("\t\"insert-s senderID deliveryID city departmentNumber\" - add new delivery\n");
	printf("\t\"update-m id name surname phone\" - update sender, enter \"-\" and \"-1\" if you want to leave field the same\n");
	printf("\t\"update-s deliveryID city departmentNumber\" - update new delivery\n");
	printf("\t\"get-m id\" - get sender by id\n");
	printf("\t\"get-m-all\" - get all senders by id\n");
	printf("\t\"get-s id\" - get delivery by id\n");
	printf("\t\"get-s-all\" - get all deliveries by id\n");
	printf("\t\"del-m id\" - delete sender (with subrecords) by id\n");
	printf("\t\"del-s deliveryID\" - delete delivery by id\n");

}