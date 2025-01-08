/*Julian Della Guardia*/

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <secret_key.h>
#include <mesh_radio.h>
#include <address.h>

void interruptHandler(void);
void timer_handler(int signum);

void encryption(uint8_t *data, uint8_t dataSize);
void updateWeight(uint8_t address);
uint8_t checkTrusted(uint8_t address);
void saveReceivedData(uint8_t *data, uint8_t dataLength);
void saveRemoteNeighborTable(uint8_t address, uint8_t *neighborData, uint8_t dataLength, uint8_t packageIndex);
void sendNextPing();

// Structure to store neighbor data
typedef struct {
    uint8_t id;                         // ID (first byte of the data)
    int weight;                         // Weight of the package
    bool inUse;                        // Flag to indicate if the package slot is in use
    bool trusted;                       // Flag to indicate if the package is trusted
    uint8_t owner;                      // Set from what route the ID is the closest
    uint8_t hops;                       // Number of hops to the receiver
} Package;

// Global variables
Package packages[MAX_SENDERS];  // Array to store packages
volatile uint8_t address = 0;
uint8_t nRF_pipe[5] = NRF_PIPE;

static volatile uint8_t rxWritePointer, rxReadPointer, rxBuffer[RX_BUFFER_DEPTH];

uint8_t broadcastMessages[DEBUG_LINES][32];
uint8_t dataMessages[DEBUG_LINES][32];

// Setup for NRF communication
void radioInit(uint8_t setAddress)
{              
	nrfspiInit();                                                               // initialize SPI
	nrfBegin();                                                                 // initialize NRF module
	nrfSetRetries(NRF_RETRY_SPEED, NRF_NUM_RETRIES);		                    // set retries
	nrfSetPALevel(NRF_POWER_LEVEL);									            // set power mode
	nrfSetDataRate(NRF_DATA_RATE);									            // set data rate				
	nrfSetCRCLength(NRF_CRC_LENGTH);                                            // CRC check
	nrfSetChannel(NRF_CHANNEL);													// set channel
	nrfSetAutoAck(NRF_AUTO_ACK);												// set acknowledge
	nrfEnableDynamicPayloads();													// enable the dynamic payload size
	
	nrfClearInterruptBits();													// clear interrupt bits
	nrfFlushRx();                                                               // Flush fifo's
	nrfFlushTx();

    printf("SPI aan %d\n", nrfVerifySPIConnection());
    sleep(1);

	// Set GPIO_PIN as input
    pinMode(1, INPUT);

    // Attach interrupt handler
    if (wiringPiISR(1, INT_EDGE_FALLING, &interruptHandler) < 0) {
        printf("Failed to setup interrupt handler for NRF.\n");
    }
	
	// Opening pipes
	nrfOpenWritingPipe((uint8_t *) nRF_pipe);								
	nrfOpenReadingPipe(0, (uint8_t *) nRF_pipe);								
	nrfStartListening();
	nrfPowerUp();

    struct sigaction sa;
    struct itimerval timer;

    // Configure the timer signal handler
    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls
    sigaction(SIGALRM, &sa, NULL);

    // Configure the timer interval
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 1000 * WEIGHT_LOWER_TIME;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 1000 * WEIGHT_LOWER_TIME;

    // Start the timer
    if (setitimer(ITIMER_REAL, &timer, NULL) == -1) {
        perror("Error setting timer");
        return;
    }

    address = setAddress;                                                      //set device address

    for (int i = 0; i < MAX_SENDERS; i++) {
        packages[i].inUse = 0;
    }
}

void sendRadioData(uint8_t target_id, uint8_t* data, uint8_t dataSize, bool encrypt){
    
    if(dataSize > MAX_DATA_LENGTH) return;
    
    uint8_t sendData[32] = {0};
    sendData[0] = address;
    sendData[1] = COMMAND_DATA;
    sendData[2] = 0;
    sendData[3] = target_id;

    for (int i = 0; i < MAX_SENDERS; i++) {
        // Find what owner the target ID has
        if (packages[i].inUse && packages[i].id == target_id) {
            if(!packages[i].owner) sendData[2] = target_id;
            else sendData[2] = packages[i].owner;
            break;
        }
    }

    // If there is no route dont send anything
    if(!sendData[2]) return;

    // Encrypt data
    if(encrypt) encryption(data, dataSize);

    // Save data to be send
    for(int i = 0; i < dataSize; i++){
        sendData[i+4] = data[i];
    }

    // Send out data
    nrfStopListening();
    nrfWrite( (uint8_t *) &sendData, dataSize + 4);
    nrfStartListening();
}

// Check if there is data to be read from NRF
uint8_t canReadRadio(void){
	uint8_t wridx = rxWritePointer, rdidx = rxReadPointer;
	
	if(wridx >= rdidx)
		return wridx - rdidx;
	else
		return wridx - rdidx + RX_BUFFER_DEPTH;
	
}

// Read first received package in buffer
uint8_t readRadioMessage(uint8_t *dataLocation) {
    uint8_t numberOfBytes = 0;
    
    for(int i = 0; i < MAX_DATA_LENGTH; i++){
        uint8_t res, curSlot, nextSlot;
        
        curSlot = rxReadPointer;
        
        // Check if there is data to be read in buffer
        if(!canReadRadio()) break;
        
        res = rxBuffer[curSlot];

        nextSlot = curSlot + 1;
        if(nextSlot >= RX_BUFFER_DEPTH)
            nextSlot = 0;
        rxReadPointer = nextSlot;
        
        // If return carriage is found end the data package
        if(res == '\r') break;

        dataLocation[i] = res;
        numberOfBytes++;
    }

    // Decrypt data
    encryption(dataLocation, numberOfBytes);

    // Return the size of the data
    return numberOfBytes;
}

// Function to print all Neigbors
void printNeighbors(int maxRows, WINDOW *window) {
    int start_y = 1, start_x = 1; // Start position for the table
    int row = 0;

    // Define headers and data
    const char *headers[] = {"ID", "Hops", "Weight", "Trusted"};

    // Draw headers
    wattron(window, COLOR_PAIR(1)); // Use header color
    mvwprintw(window, start_y, start_x, "Neighbord table:");
    mvwprintw(window, start_y + 2, start_x, "%-*s%-*s%-*s%-*s",
             ID_WIDTH, headers[0],
             HOPS_WIDTH, headers[1],
             WEIGHT_WIDTH, headers[2],
             TRUSTED_WIDTH, headers[3]);
    wattroff(window, COLOR_PAIR(1));

    // Draw rows with alternating colors
    for (int i = 0; i < MAX_SENDERS; i++) {
        if (packages[i].inUse) {
            int color_pair = (row % 2 == 0) ? 2 : 3; // Alternate colors
            wattron(window, COLOR_PAIR(color_pair));
            mvwprintw(window, start_y + 3 + row, start_x, "%-*x%-*d%-*d%-*s",
                    ID_WIDTH, packages[i].id,
                    HOPS_WIDTH, packages[i].hops,
                    WEIGHT_WIDTH, packages[i].weight,
                    TRUSTED_WIDTH, packages[i].trusted == 1 ? "yes" : "no");
            wattroff(window, COLOR_PAIR(color_pair));

            row++;
        }
    }


    for (row = row; row < maxRows; row++) {
        mvwprintw(window, start_y + 3 + row, start_x, "%-*s%-*s%-*s%-*s",
                    ID_WIDTH, " ",
                    HOPS_WIDTH, " ",
                    WEIGHT_WIDTH, " ",
                    TRUSTED_WIDTH, " ");
    }
    wrefresh(window);
}

void printBroadcasts(WINDOW *window){
    char *header = "Broadcast messages:";

    mvwprintw(window, 1, (64 - strlen(header)) /2, "%s", header);
    
    for(int i = 0; i < DEBUG_LINES; i++){
        for(int j = 0; j < 32; j++){
            int color_pair = (j % 2 == 0) ? 6 : 5; // Alternate colors
            wattron(window, COLOR_PAIR(color_pair));
            mvwprintw(window, 2 + i, j*2, "%02X", broadcastMessages[i][j]);
            wattroff(window, COLOR_PAIR(color_pair));
        }
    }

    wrefresh(window);
}

void printDataMessages(WINDOW *window){
    char *header = "Data messages:";

    mvwprintw(window, 1, (64 - strlen(header)) /2, "%s", header);
    
    for(int i = 0; i < DEBUG_LINES; i++){
        for(int j = 0; j < 32; j++){
            int color_pair = (j % 2 == 0) ? 6 : 5; // Alternate colors
            wattron(window, COLOR_PAIR(color_pair));
            mvwprintw(window, 2 + i, j*2, "%02X", dataMessages[i][j]);
            wattroff(window, COLOR_PAIR(color_pair));
        }
    }

    wrefresh(window);
}

// Function to return array of ids that belong to our network
uint8_t getOwnIds(uint8_t *ids){
    uint8_t numberOfIds = 0;

    for (int i = 0; i < MAX_SENDERS; i++) {
        if(packages[i].inUse && packages[i].trusted &&
        packages[i].id >= BASE_ADDRESS && packages[i].id <= TEST_END_ADDRESS){
            ids[numberOfIds] = packages[i].id;
            numberOfIds++;
        }
    }

    return numberOfIds;
}

// Function for both encrypting and decrypting
void encryption(uint8_t *data, uint8_t dataSize){
    uint8_t key[] = SECRET_KEY;
    for (uint8_t i = 0; i < dataSize; ++i) {
        data[i] ^= key[i % sizeof(key)];
    }
}




//##### Start of code for receiving packages #############################################################################################################

// Interrupt from NFR IC
void interruptHandler(void) 
{
	uint8_t txDs, maxRt, rxDr;
	uint8_t packetLength;
    static uint8_t received_packet[32];				    // Create a place to store received data
	
	nrfWhatHappened(&txDs, &maxRt, &rxDr);

	if ( rxDr ) {
		packetLength = nrfGetDynamicPayloadSize();	    // Get the size of the received data
		nrfRead(received_packet, packetLength);	        // Store the received data

        updateWeight(received_packet[0]);
        // mvprintw(0, 30, "%02X\n", received_packet[1]);
        if(!checkTrusted(received_packet[0])) return;

        // Check what command has been send with data
        switch (received_packet[1]) {
        // In case the data was a ping save the data in neighbor table
        case COMMAND_PING:
        case COMMAND_PING_END:
            saveRemoteNeighborTable(received_packet[0], &received_packet[3], packetLength - 2, received_packet[2]);

            memmove(&broadcastMessages[1], &broadcastMessages[0], 9 * 32);
            memcpy(broadcastMessages[0], received_packet, 32);
            break;
        
        // In case the data was sensor data send it over if the data was ment for me and otherwise save it in buffer
        case COMMAND_DATA:
            if(received_packet[2] == address){
                if(received_packet[3] != address) sendRadioData(received_packet[3], received_packet + 4, packetLength - 4, false); 
                else saveReceivedData(received_packet + 4, packetLength - 4);
            }

            memmove(&dataMessages[1], &dataMessages[0], 9 * 32);
            memcpy(dataMessages[0], received_packet, 32);
            break;

        default:
            break;
        }

	}

    memset(received_packet, 0, 32);
}

// Updata weights of direct neighbors
void updateWeight(uint8_t updateAddress){
    // Look if package id is already stored and if so update weight
    for (int i = 0; i < MAX_SENDERS; i++) {
        if (packages[i].inUse && packages[i].id == updateAddress) {
            // If package exists, just update the weight
            packages[i].weight += 10;  // Increment weight by 10
            packages[i].owner = 0;      // Set closted device as itself
            packages[i].hops  = 1;      // Set number of hops to one
            if(packages[i].weight > WEIGHT_MAX) packages[i].weight = WEIGHT_MAX;
            return;  // Done, no need to continue
        }
    }

    // If the package isnt in the neighbor table yet put it in.
    for (int i = 0; i < MAX_SENDERS; i++) {
        if (!packages[i].inUse) {
            // If an empty slot is found, store the new package
            packages[i].id = updateAddress;
            packages[i].weight = 10;    // Initial weight
            packages[i].inUse = 1;     // Mark as in use
            packages[i].owner = 0;      // Set closted device as itself
            packages[i].hops  = 1;      // Set number of hops to one
            return;
        }
    }
}

// Check if address is in trusted list
uint8_t checkTrusted(uint8_t checkAddress){
    for (int i = 0; i < MAX_SENDERS; i++) {
        if (packages[i].id == checkAddress) {
            if(packages[i].trusted) return 1;
            return 0;
        }
    }

    return 0;
}

// Save the received data in a receive buffer
void saveReceivedData(uint8_t *data, uint8_t dataLength){
    for(int i = 0; i < (dataLength + 1); i++){
        uint8_t curSlot, nextSlot;
        
        curSlot = rxWritePointer;

        // Save data in buffer and put a return carriage at the end of the message
        if(i == dataLength) rxBuffer[curSlot] = '\r';
        else rxBuffer[curSlot] = data[i];
        
        // Move over to the next slot and if the buffer depth has been reached loop back around
        nextSlot = curSlot + 1;
        if(nextSlot >= RX_BUFFER_DEPTH)
        nextSlot = 0;
        
        // Prevent overriding none read data
        if(nextSlot != rxReadPointer)
        rxWritePointer = nextSlot;
    }
}
//##### End of code for receiving packages ################################################################################################################



//##### Start of code for receiving neighbor tables #######################################################################################################

// Structure to store the sender's state
typedef struct {
    uint8_t currentMessageNumber;
    uint8_t totalMessageCount;
    uint8_t data[MAX_DATA_LENGTH];
    uint16_t dataLength;
} SenderState;

// Array to store sender states
SenderState senders[MAX_SENDERS];

// Slot occupancy array for managing sender slots
bool slotOccupied[MAX_SENDERS];

// Array to map addresses to senders
uint8_t addressMapping[MAX_SENDERS];

// Function to reset the sender's state
void resetSenderState(SenderState *sender) {
    sender->currentMessageNumber = 0;
    sender->totalMessageCount = 0;
    sender->dataLength = 0;
}

// Function to drop a sender's address from the mapping and reset its state
void dropSender(uint8_t dropAddress) {
    for (int i = 0; i < MAX_SENDERS; i++) {
        if (slotOccupied[i] && addressMapping[i] == dropAddress) {
            // Reset the sender's state
            resetSenderState(&senders[i]);
            // Mark the slot as unoccupied
            slotOccupied[i] = false;
            return;
        }
    }
}

// Function to process the completed data
void processData(uint8_t *data, uint16_t dataLength, uint8_t owner) {
    // Remove old values from table
    for (int i = 0; i < MAX_SENDERS; i++) {
        if(packages[i].inUse && packages[i].owner == owner) {
            packages[i].inUse = 0;
        }
    }
    
    // Parse the data into own neighbor table
    for (uint16_t i = 0; i < dataLength; i += 2) {
        uint8_t id = data[i];
        uint8_t hops = data[i + 1];

        if (id == 0x00) break;          // End of valid data, stop parsing
        if (id == address) continue;    // If ID is myself continue
        if (hops >= MAX_HOPS) continue; // If hops amount is above maximum amount of hops continue

        // Store the ID and hops pair
        for (int i = 0; i < MAX_SENDERS; i++) {
            if (packages[i].inUse && packages[i].id == id) {
                // If package exists, just update the hops if the hops are lower
                if(packages[i].hops > hops + 1 || !packages[i].trusted) {
                    packages[i].owner = owner;
                    packages[i].hops = hops + 1;
                }
                return;  // Done, no need to continue
            }
        }

        // If the package isnt in the neighbor table yet put it in.
        for (int i = 0; i < MAX_SENDERS; i++) {
            if (!packages[i].inUse) {
                // If an empty slot is found, store the new package
                packages[i].id = id;
                packages[i].inUse = 1;              // Mark as in use
                packages[i].trusted = 1;            // Set trusted as true since that should be handeld by sender
                packages[i].owner = owner;          // Set closted device as itself
                packages[i].hops  = hops + 1;       // Set number of hops to one
                return;
            }
        }
    }
}

// Function to handle incoming packages
void saveRemoteNeighborTable(uint8_t address, uint8_t *neighborData, uint8_t dataLength, uint8_t packageIndex) {
    int senderIndex = -1;

    // Search for the sender's address in the address mapping
    for (int i = 0; i < MAX_SENDERS; i++) {
        if (slotOccupied[i] && addressMapping[i] == address) {
            senderIndex = i;
            break;
        }
    }

    if (senderIndex == -1) {
        // New sender, find an empty slot
        for (int i = 0; i < MAX_SENDERS; i++) {
            if (!slotOccupied[i]) {
                addressMapping[i] = address;
                senderIndex = i;
                slotOccupied[i] = true;
                resetSenderState(&senders[i]);
                break;
            }
        }
    }

    if (senderIndex == -1) return; // No available space for new senders

    SenderState *sender = &senders[senderIndex];
    uint8_t currentMessageNumber = packageIndex >> 4; // First 4 bits are message number
    uint8_t totalMessageCount = packageIndex & 0x0F; // Last 4 bits are total message count

    // Check if this is the first package, and if so, initialize sender state
    if (sender->currentMessageNumber == 0) {
        sender->currentMessageNumber = currentMessageNumber;
        sender->totalMessageCount = totalMessageCount;
    }

    // If the package number or total count doesn't match, ignore it
    if (currentMessageNumber != sender->currentMessageNumber || totalMessageCount != sender->totalMessageCount) return;

    // Add the data to the sender's state
    for (uint8_t i = 0; i < dataLength; i++) {
        if (sender->dataLength < MAX_DATA_LENGTH) {
            sender->data[sender->dataLength++] = neighborData[i];
        }
    }

    // Check if we have received all expected packages
    if (sender->dataLength >= MAX_DATA_LENGTH) {
        // Process completed data once the package is full
        processData(sender->data, sender->dataLength, address);

        // Reset the sender's state after processing
        resetSenderState(sender);
    }
}
//##### End of code for receiving neighbor tables #######################################################################################################



//##### Start of code for lowering weights and pings ######################################################################################################

// Interrupt from timer counter for weight lowering and sending pings
void timer_handler(int signum) 
{
    for (int i = 0; i < MAX_SENDERS; i++) {
        // Decrease weight
        if (packages[i].weight > 0) packages[i].weight--;

        if (packages[i].inUse && !packages[i].owner) {
            // Remove package if weight is 0
            if (packages[i].weight == 0) {
                packages[i].inUse = 0; // Mark as unused
                dropSender(packages[i].id);    // Drop ID from the neighbor table handling list
            }

            // Check if weight exceeds threshold for trusted status
            else if (packages[i].weight > WEIGHT_THRESHOLD) packages[i].trusted = 1; // Mark as trusted
            
            else {
                packages[i].trusted = 0; // Reset trusted flag if below threshold
            
                // Remove all childeren if sender isnt trusted
                for (int j = 0; j < MAX_SENDERS; j++) {
                    if (packages[j].inUse && packages[j].owner == packages[i].id) {
                        packages[j].inUse = 0;
                    }
                }
            }
        }
    }
    sendNextPing();
}

// Snapshot structure
typedef struct {
    uint8_t *messages[MAX_SENDERS]; // Array of pointers to messages
    uint8_t messageLengths[MAX_SENDERS]; // Length of each message
    uint8_t totalMessages; // Total number of messages
    uint8_t currentMessage; // Index of the next message to send
} Snapshot;

Snapshot snapshot = { .totalMessages = 0, .currentMessage = 0 };

// Create snapshot from packages for neighbord table
void createSnapshot(Package packages[]) {
    snapshot.totalMessages = 0;
    snapshot.currentMessage = 0;

    uint8_t buffer[MAX_DATA_LENGTH]; // Temporary buffer
    uint8_t bufferIndex = 0;

    for (uint8_t i = 0; i < MAX_SENDERS; i++) {
        if (packages[i].inUse && packages[i].trusted) {
            // Add id and hops to the buffer
            buffer[bufferIndex++] = packages[i].id;
            buffer[bufferIndex++] = packages[i].hops;

            // If buffer is full, store the message
            if (bufferIndex >= MAX_DATA_LENGTH) {
                snapshot.messages[snapshot.totalMessages] = malloc(bufferIndex);
                memcpy(snapshot.messages[snapshot.totalMessages], buffer, bufferIndex);
                snapshot.messageLengths[snapshot.totalMessages] = bufferIndex;
                snapshot.totalMessages++;
                bufferIndex = 0;
            }
        }
    }

    // Store any remaining data in the buffer
    if (bufferIndex > 0) {
        snapshot.messages[snapshot.totalMessages] = malloc(bufferIndex);
        memcpy(snapshot.messages[snapshot.totalMessages], buffer, bufferIndex);
        snapshot.messageLengths[snapshot.totalMessages] = bufferIndex;
        snapshot.totalMessages++;
    }
}

// Send the next message in the snapshot with the index and total number in a separate variable
void sendNextPing() {
    if (snapshot.currentMessage >= snapshot.totalMessages) createSnapshot(packages);

    if (snapshot.totalMessages > 0) {
        uint8_t *message = snapshot.messages[snapshot.currentMessage];
        uint8_t messageLength = snapshot.messageLengths[snapshot.currentMessage];

        // Pack the index and total number into a separate index byte
        uint8_t indexByte = (snapshot.currentMessage << 4) | (snapshot.totalMessages & 0x0F);

        // Decide what command needs to send
        uint8_t command = 0;
        if(snapshot.currentMessage + 1 == snapshot.totalMessages) command = COMMAND_PING_END;
        else command = COMMAND_PING;

        // Send the message data
        uint8_t sendData[32] = {0};
        sendData[0] = address;
        sendData[1] = command;
        sendData[2] = indexByte;
        for(int i = 0; i < messageLength; i++){
            sendData[i+3] = message[i];
        }

        // Send out data
        nrfStopListening();
        nrfWrite( (uint8_t *) &sendData, messageLength + 4);
        nrfStartListening();

        snapshot.currentMessage++;
    }
    // If there are no neighbors just send empty ping
    else{
        uint8_t sendData[32] = {0};
        sendData[0] = address;
        sendData[1] = COMMAND_PING_END;
        sendData[2] = 0x01;

        // Send out data
        nrfStopListening();
        nrfWrite( (uint8_t *) &sendData, 4);
        nrfStartListening();
    }
}
//##### end of code for lowering weights and pings ######################################################################################################