
/**
 @name: elevator project
 @brief: terminal-based system that replicates standard elevator operation
 **/
#include <iostream>
using namespace std;

//Global variables and signals
int TIME = 0;
int START_TIME = 0;
int DEFAULT_FLOOR = 2; //changes to second floor at 2pm
int CURRENT_FLOOR = 2; //current floor of elevator. (0 = ground, 1 = 1st. 2 = 2nd, 3 = 3rd)
int callButton = 0; //call buttons 0-5
int floorButton; //inside elevator buttons 0-3
int IRon = 0; //IR signal. (0 = no signal, 1 = signal passed)
int numB = 0; //stores button input and then adds it to the x20 register
int DOORstate = 0; // 0 = closed, 1 = open
bool signalSound = false;
bool loopTrigger = true; //triggers the do-while loop
bool x19[7]; //array for floor light storage
int x20[5];   //array for floor requests
int DOOR_LATCH = 0;


//Function intializations
void movement(int &nextFloor);
void door();
void start();
void irSignal();
void callConv(int button);
void doorLightClear();
void timeOut();
void insideElevator(bool &loopTrigger);
void buttonInput();
void decoderx20();
void sortUp();
void sortDown();
bool fireKeyTest(int pos);
void fireMode();
void movementF(int &nextFloor);
void insideElevatorF();

int main(){
    start();
    return 0;
}

/**
 @brief: starts the elevator code
 @output: links to all other functions.
 **/
void start(){
    cout << "Enter input time (sec): ";
    cin >> START_TIME;
    if(START_TIME >= 50400){ //changes default floor at 2pm
        DEFAULT_FLOOR = 1;
    }
    //CURRENT_FLOOR = DEFAULT_FLOOR;
    cout << endl;
    cout << "Select call button"       << endl;
    cout << "0) ground floor - up, "   << endl;
    cout << "1) first floor - down, "  << endl;
    cout << "2) first floor - up, "    << endl;
    cout << "3) second floor - down, " << endl;
    cout << "4) second floor - up, "   << endl;
    cout << "5) third floor - down: " << endl;
    cout << "9) firefighter mode: " << endl;
    cin >> callButton;
    cout << endl;
    callConv(callButton);
}

/**
 @brief: converts elevator call buttons (0-5) to floor locations (0-3).
 @output: sends floor to movement function, to start the next step.
 **/
void callConv(int button){
    int moveTo = 0;
    switch(button){
        case (0): // Ground floor up
        {
            moveTo = 0;
            movement(moveTo);
            break;
        }
        case (1):   // 1st Floor up.
        {
            moveTo = 1;
            movement(moveTo);
            break;
        }
        case (2): // 1st floor down.
        {
            moveTo = 1;
            movement(moveTo);
            break;
        }
        case (3): // 2nd floor up.
        {
            moveTo = 2;
            movement(moveTo);
            break;
        }
        case (4): // 2nd floor down.
        {
            moveTo = 2;
            movement(moveTo);
            break;
        }
        case (5): // 3rd floor down.
            moveTo = 3;
            movement(moveTo);
            break;
        case (9): // Elevator mode
            x20[0] = 9;
            fireKeyTest(0);
            break;
    }
}

/**
 @brief: uses an algorithm that calculates future movements and outputs movements to "floor".
 @output: increments the CUURENT_FLOOR variable and calls "doors" to open.
 **/
void movement(int &nextFloor){
    if(CURRENT_FLOOR < nextFloor){
        while(CURRENT_FLOOR < nextFloor){
            x19[CURRENT_FLOOR + 4] = true;
            TIME = TIME + 5;
            ++CURRENT_FLOOR;
        }
    }
    else if(CURRENT_FLOOR > nextFloor){
        while(CURRENT_FLOOR > nextFloor){
            x19[CURRENT_FLOOR] = true;
            TIME = TIME + 5;
            --CURRENT_FLOOR;
        }
    }
    
    if(loopTrigger == true){
        door();
    }
    else{
        return;
    }
}

/**
 @brief: controls elevator door timing, door state, signalSound, irSignal, and door light indicators.
 @output:
 **/
void door(){
    cout << "The elevator is on floor " << CURRENT_FLOOR << "." << endl;
    TIME = TIME + 2; // Time to latch & door open
    cout << "*Door opens*" << endl;
    DOORstate = 1; // Door open
    signalSound = true;
    timeOut();
    signalSound = false;
    doorLightClear();
    irSignal();
    cout << "*Door closes*" << endl;
    TIME = TIME + 1; //Time to close door
    DOORstate = 0; //Door closed
    timeOut();
    insideElevator(loopTrigger);
}

/**
 @brief: outputs to user, asking if the ir was triggered. If so it will run again, adding the correct time.
 **/
void irSignal(){
    do{
        timeOut();
        cout << "Did anything trip the IR sensor? (0 = no signal, 1 = signal passed) ";
        cin >> IRon;
        cout << endl;
        if(IRon == 1){
            TIME = TIME + 1;
        }
    } while (IRon != 0);
    TIME = TIME + 2;
}

/**
 @brief: ask for inside elevator inputs, maximum of 5 inputs. 9 being firefighter key.
 **/
void insideElevator(bool &loopTrigger){
    int waitTime = 0;
    cout << "Wait time for input: ";
    cin >> waitTime;
    cout << endl;
    TIME = TIME + waitTime;
    if( waitTime >= 30){
        loopTrigger = false;
        movement(DEFAULT_FLOOR);
        timeOut();
    }
    if(waitTime < 30){
        loopTrigger = true;
        cout << "How many button inputs? ";
        cin >> numB;
        cout << endl;
        for( int i = 0; i < numB; ++i){
            buttonInput();
        }
    }
    
}

void buttonInput (){
    int butVal = 0; //# of inputs.
    for(int i = 0; i < numB; ++i){
        cout << "Enter floor button input (0-3 for floors or 9 for firefighter mode) : ";
        cin >> butVal;
        cout << endl;
        x20[i] = butVal; //inputs signals into array x20
        if (x20[i]==9){
            fireMode();
            cout<< "----FIRE MODE DISABLED----" <<endl;
            numB = 0; //clears input
            door(); //resumes normal operation of elevator
        }
        else
            x20[i] = butVal;
    }
    
    decoderx20();
    
}

//decides which direction the elevator is heading and organizes the stops.
void decoderx20(){
    if((callButton%2)==0){
        sortUp();
    }
    else if((callButton%2)==1){
        sortDown();
    }
}

void sortUp(){
    int temp = 0;
    int temp2 = 0;
    for( int i = 0; i < numB; ++i){
        for( int j = i +1; j < numB ; ++j){
            if(x20[i]>x20[j]){
                temp = x20[i];
                x20[i] = x20[j];
                x20[j] = temp;
            }
        }
    }
    fireKeyTest(numB); // test if fire key is inserted.
    for( int x = 0; x < numB; ++x){
        temp2 = x20[x];
        movement(temp2);
    }
}

void sortDown(){
    int temp = 0;
    int temp2 = 0;
    for( int i = 0; i < numB; ++i){
        for( int j = i +1; j < numB; ++j){
            if(x20[i]<x20[j]){
                temp = x20[i];
                x20[i] = x20[j];
                x20[j] = temp;
            }
        }
    }
    fireKeyTest(0); // test if fire key is inserted.
    for( int x = 0; x < numB; ++x){
        temp2 = x20[x];
        movement(temp2);
    }
}

void doorLightClear(){
    for(int i = 0; i < 7; i++){
        x19[i] = false;
    }
}

/**
 @brief: outputs signal changes and time.
 **/
void timeOut () {
    int curTime = 0;
    curTime = TIME + START_TIME;
    cout << "-------------------------------------------" << endl;
    cout << "Current time (sec):     " << curTime << endl;
    cout << "Run time (sec):         " << TIME << endl;
    cout << "-------------------------------------------" << endl << endl;
    cout << "Current Floor:          " << CURRENT_FLOOR << endl;
    cout << "Door State:             " << DOORstate << endl;
    cout << "IR Signal:              " << IRon << endl;
    cout << "Door Sound:             " << signalSound << endl;
    for(int i = 0; i < 4; i++){
        cout << "Down Light (" << i << "):         ";
        cout << x19[i] << endl;
    }
    for(int i = 4; i < 8; i++){
        cout << "Up Light   (" << i-4 << "):         ";
        cout << x19[i] << endl;
    }
}

//tests if the firefighter key has been inserted.
bool fireKeyTest(int pos) { //pos is location of 9 in the array
    bool test = false;
    if(x20[pos]==9){
        fireMode();
        test = true;  //returns true if 'firemode' was not ran.
    }
    else if(x20[pos]!=9){
        test = false; //returns false if 'firemode' was not ran.
    }
    return test;
}

void fireMode(){
    cout << "!***FIRE MODE ENABLED***!" << endl;
    for(int i = 0; i < 5; ++i){ //clears array to zero
        x20[i] = 0;
    }
    int key = 0; //floor the key was inserted on.
    cout << "What floor was the fire key inserted? ";
    cin >> key;
    cout << endl;
    movementF(key);
}

//movement for elevator in firefighter mode.
void movementF(int& nextFloor){
    int fireKey = 1;
    cout << "Is the firefighter key still inserted? (0 = false, 1 = true)" << endl;
    cin >> fireKey;
    if(fireKey == 1){
        
        if(CURRENT_FLOOR < nextFloor){
            while(CURRENT_FLOOR < nextFloor){
                TIME = TIME + 6;
                DOOR_LATCH = 1;
                ++CURRENT_FLOOR;
            }
        }
        else if(CURRENT_FLOOR > nextFloor){
            while(CURRENT_FLOOR > nextFloor){
                TIME = TIME + 6;
                DOOR_LATCH = 1;
                --CURRENT_FLOOR;
            }
        }
        DOOR_LATCH = 0;
        int fireWait = 0;
        timeOut();
        cout << "Wait time for opening the door: ";
        cin >> fireWait;
        cout  << endl;
        cout << "Door has been manually opened." << endl;
        DOORstate = 1;
        TIME = TIME + fireWait;
        timeOut();
        insideElevatorF();
    }
    else if(fireKey == 0){
        return;
    }
}

//inside elevator for firefighter mode
void insideElevatorF(){
    int waitTime = 0;
    int fCloseDoor = 0;
    int doorB = 0;
    int fireFloor = 0;
    cout << "Wait time for firefighter's input: ";
    cin >> waitTime;
    cout << endl;
    cout << "Wait time for firefighter to close door: ";
    cin >> fCloseDoor;
    cout << endl;
    cout << "Is door manually closed? (0 = closed, 1 = open) ";
    cin >> doorB;
    cout << endl;
    cout << "Firefighter input floor: " << endl;
    cin >> fireFloor;
    if(doorB == 0){
        DOORstate = 0;
    }
    else if(doorB == 1){
        DOORstate = 1;
    }
    timeOut();
    TIME = TIME + waitTime + fCloseDoor;
    movementF(fireFloor);
}
