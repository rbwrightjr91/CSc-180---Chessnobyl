/*
Richard Wright
Chessnobyl
"Newclear" Chess Project
CSc 180
Spring 2018
*/

#include <iostream>
#include <stdlib.h>
#include <time.h>
using namespace std;

const int LEGAL_MOVE_SIZE = 300;

const int EMPTY_SPACE = 0;
const int OOB_SPACE = 99;

//computer pieces
const int CPU_P = 1;
const int CPU_R = 2;
const int CPU_N = 3;
const int CPU_B = 4;
const int CPU_K = 5;

//human pieces
const int HUM_P = -1;
const int HUM_R = -2;
const int HUM_N = -3;
const int HUM_B = -4;
const int HUM_K = -5;

int maxDepth, moveNum;
int minKillerMove[2];
int maxKillerMove[2];
bool humTurn, humMoveFirst;

void setup();
void dispBoard();
void humMove();
void cpuMove();
void minMax(int (&bestMv)[2], clock_t &timer);
int min(int depth, int alpha, int beta, clock_t &timer);
int max(int depth, int alpha, int beta, clock_t &timer);
int heur();
int cpuStateEval();
int humStateEval();
void makeMove(int from, int to);
bool tryMove(int from, int to, int (&array)[10]);
void retractMove(int to, int from, int (&array)[10]);
int winCheck(bool isHumTurn);
void gameOverCheck(bool isHumTurn);
void moveToFront(int from, int to, int (&array)[LEGAL_MOVE_SIZE]);
void legalMoveGenerator(int (&array)[LEGAL_MOVE_SIZE]);
int boardSpaceToArraySpace(string boardSpace);
string arraySpaceToBoardSpace(int arraySpace);
string mirror(int arraySpace);
void welcome();


//board[row][col]
int b[117];

int main() {
    welcome();
    setup();
    dispBoard();
    for(;;) {
        if(humMoveFirst){
            humMove();
            gameOverCheck(humTurn);          
            cpuMove();
            gameOverCheck(humTurn);
        }else if(!humMoveFirst){
            cpuMove();
            gameOverCheck(humTurn);
            humMove();
            gameOverCheck(humTurn);
        }
    }
}

void setup() {

    char turn;

    for(;;){
        cout << "\033[1;33mWould\033[0m \033[1;32myou\033[0m \033[1;33mlike to play first (1) or second (2)?:\033[0m ";
        cin >> turn;

        if(turn == '1'){ humMoveFirst = true; break;}
        else if(turn == '2'){ humMoveFirst = false; break; }
        else{ cout << "Invalid input: " << turn << endl; continue; }
    }

    humTurn = humMoveFirst;
    moveNum = 0;

    for(int i=0; i < 117; i += 9) {
        for(int j = 0; j < 9; j++) {
            if(i < 18 || i > 90) { b[i+j] = OOB_SPACE; }
            else {
                switch(j) {
                    case 0:
                    case 8:
                        b[i + j] = OOB_SPACE;
                        break;
                    default:
                        b[i+j] = EMPTY_SPACE;
                }
            }
        }
    }

    //set cpu pieces
    b[20] = b[24] = CPU_N;
    b[21] = b[23] = CPU_R;
    b[22] = CPU_K;
    b[31] = b[40] = CPU_B;
    b[46] = b[48] = b[50] = b[52] = CPU_P;

    //set human pieces
    b[64] = b[66] = b[68] = b[70] = HUM_P;
    b[76] = b[85] = HUM_B;
    b[94] = HUM_K;
    b[93] = b[95] = HUM_R;
    b[92] = b[96] = HUM_N;
    
}

void dispBoard() {

    int r = 10;

    cout << endl;
    cout << "\033[1;33m   A B C D E F G\033[0m" << endl;    
    for(int i = 18; i < 99; i += 9) {
        cout << "\033[1;33m " << --r << " \033[0m";

        for(int j=0; j < 9; j++){
            switch (b[i+j]) {
                case EMPTY_SPACE:{ if((i+j)%2 == 0){ cout << "\033[1;33m+\033[0m" << ' '; break; } else { cout << "\033[1;36m+\033[0m" << ' '; break; } }
                case CPU_P: cout << "\033[1;31mP\033[0m" << ' '; break;
                case CPU_N: cout << "\033[1;31mN\033[0m" << ' '; break;
                case CPU_B: cout << "\033[1;31mB\033[0m" << ' '; break;
                case CPU_R: cout << "\033[1;31mR\033[0m" << ' '; break;
                case CPU_K: cout << "\033[1;31mK\033[0m" << ' '; break;
                case HUM_P: cout << "\033[1;32mp\033[0m" << ' '; break;
                case HUM_N: cout << "\033[1;32mn\033[0m" << ' '; break;
                case HUM_B: cout << "\033[1;32mb\033[0m" << ' '; break;
                case HUM_R: cout << "\033[1;32mr\033[0m" << ' '; break;
                case HUM_K: cout << "\033[1;32mk\033[0m" << ' '; break;
            }
        }
        cout << "\033[1;33m" << r << " \033[0m";
        cout << endl;
    }
    cout << "\033[1;33m   A B C D E F G\033[0m" << endl;
}

void humMove() {

    int legalMoves[LEGAL_MOVE_SIZE] = {0};
    string input, from, to;

    legalMoveGenerator(legalMoves);

    cout << endl << "Legal Moves: [";

    for (int i = 0; i < sizeof(legalMoves) / sizeof(legalMoves[0]); i += 2) {
        if (b[legalMoves[i]] < 0) {
            cout << arraySpaceToBoardSpace(legalMoves[i]) << arraySpaceToBoardSpace(legalMoves[i + 1]);
            if(i+2 < sizeof(legalMoves) / sizeof(legalMoves[0])) {cout << " ";}
        }
    }

    cout << "]" << endl;

    for(;;) {
        cout << endl << "Please make a move or (Q)uit: ";
        cin >> input;
        cout << endl;

        if (input.substr(0, 1) == "Q" || input.substr(0, 1) == "q") {
            cout << "Quitting..." << endl;
            exit(0);
        }

        if(input.length() < 4) {
            cout << "Illegal Move" << endl;
            continue;
        }

        int from = boardSpaceToArraySpace(input.substr(0, 2));
        int to = boardSpaceToArraySpace(input.substr(2, 4));

        bool legal = false;

        for (int i = 0; legalMoves[i] != 0; i += 2) {
            if (b[legalMoves[i]] < 0) {
                if(from == legalMoves[i] && to == legalMoves[i+1]) {
                    legal = true; makeMove(from, to); break;
                }             
            }
        }

        if(legal) { break; }
        else {cout << endl << "Illegal Move" << endl;}
    }

    humTurn = !humTurn;
    
}

void cpuMove() {
    clock_t timer = clock();
    maxDepth = 1;
    int bestMv[2] = {0}, lastDepthMv[2] = {0};
    for(;;) {
        if((clock() - timer)/CLOCKS_PER_SEC >= 5) { break; }
        minMax(bestMv, timer);
        if(bestMv[0] != 1234567) {
            lastDepthMv[0] = bestMv[0];
            lastDepthMv[1] = bestMv[1];
            maxDepth++;
        }else { break; }
    }

    if(bestMv[0] == 1234567) {
        bestMv[0] = lastDepthMv[0];
        bestMv[1] = lastDepthMv[1];
    }

    cout << endl << "\033[1;31mMove: " << arraySpaceToBoardSpace(bestMv[0]) << arraySpaceToBoardSpace(bestMv[1]) << " (" << mirror(bestMv[0]) << mirror(bestMv[1]) << ")" << endl;

    makeMove(bestMv[0], bestMv[1]);

    cout << "Move number: " << ++moveNum << endl << "Move time: " << float (clock() - timer)/CLOCKS_PER_SEC << " second(s)\033[0m" << endl;

    humTurn = !humTurn;
}


void minMax(int (&bestMv)[2], clock_t &timer) {
    int legalMoves[LEGAL_MOVE_SIZE] = {0}, oldTo[10];
    int bestSc = -9999, alpha = -9999, beta = 9999, depth = 0, score;

    legalMoveGenerator(legalMoves);

    for(int i = 0; legalMoves[i] != 0; i+=2) {
        if (b[legalMoves[i]] > 0) {
            tryMove(legalMoves[i], legalMoves[i+1], oldTo);
            score = min(depth + 1, alpha, beta, timer);
            if(score == 1234567) { bestMv[0] = bestMv[1] = score; }
            if(score > bestSc) { bestSc = score; bestMv[0] = legalMoves[i]; bestMv[1] = legalMoves[i+1]; }
            else if (score == bestSc) {
                if (legalMoves[i+1] == -5){ bestMv[0] = legalMoves[i]; bestMv[1] = legalMoves[i+1]; }
                else { 
                    for(int i = 0; i < sizeof(oldTo)/sizeof(oldTo[0]); i++) {
                        if(oldTo[i] == -5) {
                            bestMv[0] = legalMoves[i]; bestMv[1] = legalMoves[i+1];
                            break; 
                        }
                    }
                }
            }
            retractMove(legalMoves[i+1], legalMoves[i], oldTo);
        }
    }
}

int min(int depth, int alpha, int beta, clock_t &timer) {
    int win = winCheck(true);
    if(win != 0) { if(win < 0) { return win + depth; }else { return win - depth; } }
    if(depth == maxDepth) { return heur(); }
    if(float (clock() - timer)/CLOCKS_PER_SEC >= 5) { return 1234567; }

    int minSc = 9999, score, oldTo[10] = {0}, legalMoves[LEGAL_MOVE_SIZE] = {0};
    legalMoveGenerator(legalMoves);

    for(int i = 0; legalMoves[i] != 0; i+=2){
        if(legalMoves[i] == minKillerMove[0] && legalMoves[i+1] == minKillerMove[1]){
            moveToFront(legalMoves[i], legalMoves[i+1], legalMoves);
        }
    }

    for(int i = 0; legalMoves[i] != 0; i+=2) {
        if (b[legalMoves[i]] < 0) {
            bool capture = tryMove(legalMoves[i], legalMoves[i+1], oldTo);
            score = max(depth + 1, alpha, beta, timer);
            if(score < minSc) { minSc = score; }
            if(minSc < beta) { beta = minSc; }
            retractMove(legalMoves[i+1], legalMoves[i], oldTo);
            if(beta <= alpha) {
                minKillerMove[0] = legalMoves[i];
                minKillerMove[1] = legalMoves[i+1];
                break;
            }
        } 
    }    

    return minSc;
}

int max(int depth, int alpha, int beta, clock_t &timer) {
    int win = winCheck(false);
    if(win != 0) { if(win > 0) { return win - depth; }else { return win + depth; } }
    if(depth == maxDepth) { return heur(); }
    if(float (clock() - timer)/CLOCKS_PER_SEC >= 5) { return 1234567; }

    int maxSc = -9999, score, oldTo[10] = {0}, legalMoves[LEGAL_MOVE_SIZE] = {0};
    legalMoveGenerator(legalMoves);

    for(int i = 0; legalMoves[i] != 0; i+=2){
        if(legalMoves[i] == maxKillerMove[0] && legalMoves[i+1] == maxKillerMove[1]){
            moveToFront(legalMoves[i], legalMoves[i+1], legalMoves);
        }
    }

    for(int i = 0; legalMoves[i] != 0; i+=2) {
        if (b[legalMoves[i]] > 0) {
            tryMove(legalMoves[i], legalMoves[i+1], oldTo);
            score = min(depth + 1, alpha, beta, timer);
            if(score > maxSc) { maxSc = score; }
            if(maxSc > alpha) { alpha = maxSc; }
            retractMove(legalMoves[i+1], legalMoves[i], oldTo);
            if(beta <= alpha) { 
                maxKillerMove[0] = legalMoves[i];
                maxKillerMove[1] = legalMoves[i+1];
                break;
            }
        } 
    }    

    return maxSc;
}

int heur() {
    return cpuStateEval() + humStateEval();
}

int cpuStateEval() {
    int mobility = 0, pieceWeight = 0, pieceCount = 0, legalMoves[LEGAL_MOVE_SIZE] = {0}, pieceType[5] = {0};
    legalMoveGenerator(legalMoves);

    for(int i = 0; legalMoves[i] != 0; i+=2) {
        if (b[legalMoves[i]] > 0) {
            pieceCount++;
            mobility++;
            pieceType[b[legalMoves[i]] - 1]++;
        }
    }

    for(int i = 0; i < 5; i++){
        pieceWeight += (i+1)*(i+1) * pieceType[i];
    }

    return  mobility + pieceWeight;
}

int humStateEval() {
    int mobility = 0, pieceWeight = 0, pieceCount = 0, legalMoves[LEGAL_MOVE_SIZE] = {0}, pieceType[5] = {0};
    legalMoveGenerator(legalMoves);

    for(int i = 0; legalMoves[i] != 0; i+=2) {
        if (b[legalMoves[i]] < 0) {
            pieceCount--;
            mobility--;
            pieceType[b[legalMoves[i]] * -1 - 1]--;
        }
    }

    for(int i = 0; i < 5; i++){
        pieceWeight += (i+1)*(i+1) * pieceType[i];
    }

    return mobility + pieceWeight;
}

void makeMove(int from, int to) {
    if(from != to){
        b[to] = b[from];
        b[from] = EMPTY_SPACE;
    }else {
        b[from] = EMPTY_SPACE;
        (b[from+8] != 99)? b[from+8] = EMPTY_SPACE : OOB_SPACE;
        (b[from+9] != 99)? b[from+9] = EMPTY_SPACE : OOB_SPACE;
        (b[from+10] != 99)? b[from+10] = EMPTY_SPACE : OOB_SPACE;
        (b[from+1] != 99)? b[from+1] = EMPTY_SPACE : OOB_SPACE;
        (b[from-10] != 99)? b[from-10] = EMPTY_SPACE : OOB_SPACE;
        (b[from-9] != 99)? b[from-9] = EMPTY_SPACE : OOB_SPACE;
        (b[from-8] != 99)? b[from-8] = EMPTY_SPACE : OOB_SPACE;
        (b[from-1] != 99)? b[from-1] = EMPTY_SPACE : OOB_SPACE;
    }
}

bool tryMove(int from, int to, int (&array)[10]) {
    bool capture = false;
    if(from != to){

        if(b[from] > 0 && b[to] > 0) {
            capture = true;
        }else if(b[from] < 0 && b[to] > 0) {
            capture = true;
        }

        array[0] = 0;
        array[1] =  b[from]; 
        
        for(int i = 2; i < sizeof(array)/sizeof(array[0]); i++){
            array[i] = b[to];
        }

        b[to] = b[from];
        b[from] = EMPTY_SPACE;
    }else {
        int fp = b[from];

        array[0] = 1;
        array[1] = b[from];
        array[2] = b[from+8];
        array[3] = b[from+9];
        array[4] = b[from+10];
        array[5] = b[from+1];
        array[6] = b[from-8];
        array[7] = b[from-9];
        array[8] = b[from-10];
        array[9] = b[from-1];
        b[from] = EMPTY_SPACE;
        (b[from+8] != 99)? b[from+8] = EMPTY_SPACE : OOB_SPACE;
        (b[from+9] != 99)? b[from+9] = EMPTY_SPACE : OOB_SPACE;
        (b[from+10] != 99)? b[from+10] = EMPTY_SPACE : OOB_SPACE;
        (b[from+1] != 99)? b[from+1] = EMPTY_SPACE : OOB_SPACE;
        (b[from-10] != 99)? b[from-10] = EMPTY_SPACE : OOB_SPACE;
        (b[from-9] != 99)? b[from-9] = EMPTY_SPACE : OOB_SPACE;
        (b[from-8] != 99)? b[from-8] = EMPTY_SPACE : OOB_SPACE;
        (b[from-1] != 99)? b[from-1] = EMPTY_SPACE : OOB_SPACE;

        for(int i = 0; i < sizeof(array)/sizeof(array[0]); i++){
            if(fp > 0 && array[i] > 0) {
                capture = true; break;
            }else if(fp < 0 && array[i] > 0) {
                capture = true; break;
            }
        }
    }

    return capture;
}

void retractMove(int to, int from, int (&array)[10]) {    
    if(array[0] == 0) {
        b[from] = array[1];
        b[to] = array[2];
    }

    if(array[0] == 1) {
        b[from] = array[1];
        b[from+8] = array[2];
        b[from+9] = array[3];
        b[from+10] = array[4];
        b[from+1] = array[5];
        b[from-8] = array[6];
        b[from-9] = array[7];
        b[from-10] = array[8];
        b[from-1] = array[9];
    }
}


int winCheck(bool isHumTurn) {
    bool hk, ck, hm, cm;
    int legalMoves[LEGAL_MOVE_SIZE] = { 0 };

    hk = ck = hm = cm = false;
    
    legalMoveGenerator(legalMoves);

    //check if there are legal moves for either side
    for(int i = 0; legalMoves[i] != 0; i+=2){
        if(!cm && b[legalMoves[i]] > 0){  cm = !cm; }
        else if(!hm && b[legalMoves[i]] < 0){ hm = !hm; }
    }

    //end game if there is no legal move to be made on current side's turn
    if(!hm && isHumTurn){ return 9999; }
    else if(!cm && !isHumTurn){ return -9999; }

    //check if the kings are on the board
    for(int i = 19; i <= 91; i += 9) {
        for(int j=0; j < 7; j++){
            if(!hk && b[i+j] == HUM_K){ hk = true; }
            else if( !ck && b[i+j] == CPU_K ){ ck = true; }            
        }
    }

    //end game if one or both kings are missing
    if(hk && ck){ return 0; }
    else if(hk && !ck){ return -9999; }
    else if(!hk && ck){ return 9999; }
    else if(!hk && !ck){ 
        if(isHumTurn){ return -9999; }
        else if(!isHumTurn){ return 9999; }
    }
}

void moveToFront(int from, int to, int (&array)[LEGAL_MOVE_SIZE]) {
    int fi = 0;

    while(array[fi] != from && array[fi+1] != to){
        fi+=2;
    }


    for(int i = fi - 2; i >= 0; i--) {
        array[i+2] = array[i];
    }

    array[0] = from;
    array[1] = to;
}

void gameOverCheck(bool isHumTurn) {
    dispBoard();

    int win = winCheck(isHumTurn);

    if(win < 0){ cout << endl << "\033[1;32mHuman win\033[0m" << endl; exit(0); }
    else if(win > 0){ cout << endl << "\033[1;31mCPU Win\033[0m" << endl; exit(0); }
}

int boardSpaceToArraySpace(string boardSpace) {
    int arraySpace = 0;

    switch(boardSpace.at(0)) {
        case 'a': case 'A': break;
        case 'b': case 'B': arraySpace++; break;
        case 'c': case 'C': arraySpace+=2; break;
        case 'd': case 'D': arraySpace+=3; break;
        case 'e': case 'E': arraySpace+=4; break;
        case 'f': case 'F': arraySpace+=5; break;
        case 'g': case 'G': arraySpace+=6; break;
    }

    switch(boardSpace.at(1)) {
        case '9': arraySpace+=19; break;
        case '8': arraySpace+=28; break;
        case '7': arraySpace+=37; break;
        case '6': arraySpace+=46; break;
        case '5': arraySpace+=55; break;
        case '4': arraySpace+=64; break;
        case '3': arraySpace+=73; break;
        case '2': arraySpace+=82; break;
        case '1': arraySpace+=91; break;
    }

    return arraySpace;
}

string arraySpaceToBoardSpace(int arraySpace) {
    string row, col;

    switch(arraySpace % 9){
        case 1: col = "A"; break;
        case 2: col = "B"; break;
        case 3: col = "C"; break;
        case 4: col = "D"; break;
        case 5: col = "E"; break;
        case 6: col = "F"; break;
        case 7: col = "G"; break;
    }

    switch(arraySpace / 9) {
        case 2: row = "9"; break;
        case 3: row = "8"; break;
        case 4: row = "7"; break;
        case 5: row = "6"; break;
        case 6: row = "5"; break;
        case 7: row = "4"; break;
        case 8: row = "3"; break;
        case 9: row = "2"; break;
        case 10: row = "1"; break;
    }

    return col + row;
}

string mirror(int arraySpace) {

    string row, col;

    switch(arraySpace % 9){
        case 1: col = "G"; break;
        case 2: col = "F"; break;
        case 3: col = "E"; break;
        case 4: col = "D"; break;
        case 5: col = "C"; break;
        case 6: col = "B"; break;
        case 7: col = "A"; break;
    }

    switch(arraySpace / 9) {
        case 2: row = "1"; break;
        case 3: row = "2"; break;
        case 4: row = "3"; break;
        case 5: row = "4"; break;
        case 6: row = "5"; break;
        case 7: row = "6"; break;
        case 8: row = "7"; break;
        case 9: row = "8"; break;
        case 10: row = "9"; break;
    }

    return col + row;
}

void legalMoveGenerator(int (&array)[LEGAL_MOVE_SIZE]) {

    int p = 0; //array pointer

    for(int i = 19; i < 100; i += 9) {
        for(int j=0; j < 7; j++){
            switch (b[i+j]) {
                case EMPTY_SPACE: break;           
                case CPU_K:
                    for(int k = 8; k <= 10; k++){
                        if(b[i+j+k] <= 0 && b[i+j+k] != 99){ array[p] = i+j; p++; array[p] = i+j+k; p++; }
                    } 
                    break;                
                case CPU_R: {
                    for(int k = i+j+9; b[k] < 99; k+=9) {
                        if(b[k] == 0) { array[p] = i+j; p++; array[p] = k; p++; }
                        else if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] > 0) { break; }
                    }

                    for(int k = i+j+1; b[k] < 99; k++) {
                        if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] > 0) { break; }
                    }

                    for(int k = i+j-1; b[k] < 99; k--) {
                        if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] > 0) { break; }
                    }

                    for(int k = i+j-9; b[k] < 99; k-=9) {
                        if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] > 0) { break; }
                    }

                    array[p] = array[p+1] = i+j; p+=2;
                    break;
                }                
                case CPU_B: {
                    for(int k = i+j+8; b[k] < 99; k+=8) {
                        if(b[k] == 0) { array[p] = i+j; p++; array[p] = k; p++; }
                        else if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] > 0) { break; }
                    }

                    for(int k = i+j+10; b[k] < 99; k+=10) {
                        if(b[k] == 0) { array[p] = i+j; p++; array[p] = k; p++; }
                        else if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] > 0) { break; }
                    }

                    for(int k = i+j-8; b[k] < 99; k-=8) {
                        if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                    }

                    for(int k = i+j-10; b[k] < 99; k-=10) {
                        if(b[k] < 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                    }

                    array[p] = array[p+1] = i+j; p+=2;
                    break;
                }                
                case CPU_N: { 
                    if(b[i+j+7] <= 0 && b[i+j+7] != 99){ array[p] = i+j; p++; array[p] = i+j+7; p++; }
                    if(b[i+j+11] <= 0 && b[i+j+11] != 99){ array[p] = i+j; p++; array[p] = i+j+11; p++; }
                    if(b[i+j+17] <= 0 && b[i+j+17] != 99){ array[p] = i+j; p++; array[p] = i+j+17; p++; } 
                    if(b[i+j+19] <= 0 && b[i+j+19] != 99){ array[p] = i+j; p++; array[p] = i+j+19; p++; }
                    if(b[i+j-7] < 0 && b[i+j-7] != 99){ array[p] = i+j; p++; array[p] = i+j-7; p++; }
                    if(b[i+j-11] < 0 && b[i+j-11] != 99){ array[p] = i+j; p++; array[p] = i+j-11; p++; }
                    if(b[i+j-17] < 0 && b[i+j-17] != 99){ array[p] = i+j; p++; array[p] = i+j-17; p++; } 
                    if(b[i+j-19] < 0 && b[i+j-19] != 99){ array[p] = i+j; p++; array[p] = i+j-19; p++; }
                    array[p] = array[p+1] = i+j; p+=2;
                    break;
                }                
                case CPU_P:
                    if(b[i+j+8] < 0 && b[i+j+8] != 99){ array[p] = i+j; p++; array[p] = i+j+8; p++; }
                    if(b[i+j+9] == 0){ array[p] = i+j; p++; array[p] = i+j+9; p++; }
                    if(b[i+j+10] < 0 && b[i+j+10] != 99){ array[p] = i+j; p++; array[p] = i+j+10; p++; }
                    array[p] = array[p+1] = i+j; p+=2; 
                    break;                
                case HUM_P:
                    if(b[i+j-8] > 0 && b[i+j-8] != 99){ array[p] = i+j; p++; array[p] = i+j-8; p++; }
                    if(b[i+j-9] == 0){ array[p] = i+j; p++; array[p] = i+j-9; p++; }
                    if(b[i+j-10] > 0 && b[i+j-10] != 99){ array[p] = i+j; p++; array[p] = i+j-10; p++; }
                    array[p] = array[p+1] = i+j; p+=2;
                    break;                
                case HUM_N: { 
                    if(b[i+j-7] >= 0 && b[i+j-7] != 99){ array[p] = i+j; p++; array[p] = i+j-7; p++; }
                    if(b[i+j-11] >= 0 && b[i+j-11] != 99){ array[p] = i+j; p++; array[p] = i+j-11; p++; }
                    if(b[i+j-17] >= 0 && b[i+j-17] != 99){ array[p] = i+j; p++; array[p] = i+j-17; p++; } 
                    if(b[i+j-19] >= 0 && b[i+j-19] != 99){ array[p] = i+j; p++; array[p] = i+j-19; p++; }
                    if(b[i+j+7] > 0 && b[i+j+7] != 99){ array[p] = i+j; p++; array[p] = i+j+7; p++; }
                    if(b[i+j+11] > 0 && b[i+j+11] != 99){ array[p] = i+j; p++; array[p] = i+j+11; p++; }
                    if(b[i+j+17] > 0 && b[i+j+17] != 99){ array[p] = i+j; p++; array[p] = i+j+17; p++; } 
                    if(b[i+j+19] > 0 && b[i+j+19] != 99){ array[p] = i+j; p++; array[p] = i+j+19; p++; }
                    array[p] = array[p+1] = i+j; p+=2;
                    break;
                }                
                case HUM_B: {
                    for(int k = i+j-8; b[k] < 99; k-=8) {
                        if(b[k] == 0) { array[p] = i+j; p++; array[p] = k; p++; }
                        else if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] < 0) { break; }
                    }

                   for(int k = i+j-10; b[k] < 99; k-=10) {
                        if(b[k] == 0) { array[p] = i+j; p++; array[p] = k; p++; }
                        else if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] < 0) { break; }
                    }

                    for(int k = i+j+8; b[k] < 99; k+=8) {
                        if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                    }

                    for(int k = i+j+10; b[k] < 99; k+=10) {
                        if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                    }
                    array[p] = array[p+1] = i+j; p+=2;
                    break;
                }               
                case HUM_R: {
                    for(int k = i+j-9; b[k] < 99; k-=9) {
                        if(b[k] == 0) { array[p] = i+j; p++; array[p] = k; p++; }
                        else if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] < 0) { break; }
                    }

                    for(int k = i+j+1; b[k] < 99; k++) {
                        if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] < 0) { break; }
                    }

                    for(int k = i+j-1; b[k] < 99; k--) {
                        if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] < 0) { break; }
                    }

                    for(int k = i+j+9; b[k] < 99; k+=9) {
                        if(b[k] > 0) { array[p] = i+j; p++; array[p] = k; p++; break; }
                        else if(b[k] < 0) { break; }
                    }
                    array[p] = array[p+1] = i+j; p+=2;
                    break;
                }                    
                case HUM_K:
                    for(int k = 8; k <= 10; k++){
                        if(b[i+j-k] >= 0 && b[i+j-k] != 99){ array[p] = i+j; p++; array[p] = i+j-k; p++; }
                    }
                    break;
                default: break;                
            }
        }
    }

}

void welcome(){

    cout << endl << endl << "                                              \033[1;36mWelcome to\033[0m" << endl;
    cout << "\033[1;31m       ______  __    __  _______     _______.  _______..__   __.  ______  .______ ____    ____ __         \033[0m" << endl;
    cout << "\033[1;31m      /      ||  |  |  ||   ____|   /       | /       ||  \\ |  | /  __  \\ |   _  \\\\   \\  /   /|  |        \033[0m" << endl;
    cout << "\033[1;31m     |  ,----'|  |__|  ||  |__     |   (----`|   (----`|   \\|  ||  |  |  ||  |_)  |\\   \\/   / |  |        \033[0m" << endl;
    cout << "\033[1;31m     |  |     |   __   ||   __|     \\   \\     \\   \\    |  . `  ||  |  |  ||   _  <  \\_    _/  |  |        \033[0m" << endl;
    cout << "\033[1;31m     |  `----.|  |  |  ||  |____.----)   |.----)   |   |  |\\   ||  `--'  ||  |_)  |   |  |    |  `----.   \033[0m" << endl;
    cout << "\033[1;31m      \\______||__|  |__||_______________/ |_______/    |__| \\__| \\______/ |______/    |__|    |_______|   \033[0m" << endl << endl;
    
    cout << "\033[1;33m============================================================================================================\033[0m" << endl << endl;

}