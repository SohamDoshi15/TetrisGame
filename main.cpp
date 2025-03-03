#include <iostream>
#include <vector>
#include <string>
using namespace std;
#include <windows.h>
#include <thread>
const int nFieldWidth = 10;
const int nFieldHeight = 20;
class Tetromino{
    private:
    wstring pieceShape;
    int pieceId;
    int positionX;
    int positionY;
    int currentRotation;

    int calculateRotation(int blockX,int blockY){
        int rotatedIndex=0;
        //currentRotation is 0,90,180 and 270 degrees
        if(currentRotation%4==0){
            rotatedIndex=blockY*4+blockX;
        }
        else if(currentRotation%4==1){
            rotatedIndex=12+blockY-(blockX*4);
        }
        else if(currentRotation%4==2){
            rotatedIndex=15-(blockY*4)-blockX;
        }
        else if(currentRotation%4==3){
            rotatedIndex=3-blockY+(blockX*4);
        }
        return rotatedIndex;
    }
    public:
    Tetromino(wstring shapeInput, int idInput){
        pieceShape=shapeInput;
        pieceId=idInput;
        positionX=nFieldWidth/2;
        positionY=0;
        currentRotation=0;
    }
    bool canFit(int newX,int newY, int newRotation,unsigned char gameField[][nFieldWidth]){
        currentRotation=newRotation;
        for(int blockX=0;blockX<4;blockX++){
            for (int blockY = 0; blockY< 4; blockY++){
                int shapeIndex=calculateRotation(blockX,blockY);
                int fieldX=newX+blockX;
                int fieldY=newY+blockY;
                if (fieldX >= 0 && fieldX < nFieldWidth){
                    if(fieldY>=0 && fieldY < nFieldHeight){
                        if(pieceShape[shapeIndex] != L'.' && gameField[fieldY][fieldX] != 0){
                            return false;
                        }
                    }
                }
            }
        }
        currentRotation=newRotation;
        return true;
    }
    wstring getPieceShape(){
        return pieceShape;
    }
    int getPieceId(){
        return pieceId;
    }
    int getPositionX(){
        return positionX;
    }
    int getPositionY(){
        return positionY;
    }
    int getCurrentRotation(){
        return currentRotation;
    }
    void setPosition(int newX, int newY){
        positionX=newX;
        positionY=newY;
    }
    void move(int changeX,int changeY){
        positionX=positionX+changeX;
        positionY=positionY+changeY;
    }
    void setRotation(int newRotation){
        currentRotation=newRotation;
    }
};

int nScreenWidth = 80;
int nScreenHeight = 30;
Tetromino structuredTetrominos[7];
unsigned char pField[nFieldHeight][nFieldWidth];

int main(){
    wchar_t *screen=new wchar_t[nScreenWidth*nScreenHeight];
    for(int i=0 ; i < nScreenWidth*nScreenHeight; i++){
        screen[i]=L' ';
    }
    HANDLE hConsole=CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0 , NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten=0;

    structuredTetrominos[0] = Tetromino(L"..X...X...X...X.", 0); // I
    structuredTetrominos[1] = Tetromino(L"..X..XX...X.....", 1); // J
    structuredTetrominos[2] = Tetromino(L".....XX..XX.....", 2); // O
    structuredTetrominos[3] = Tetromino(L"..X..XX..X......", 3); // L
    structuredTetrominos[4] = Tetromino(L".X...XX...X.....", 4); // S
    structuredTetrominos[5] = Tetromino(L".X...X...XX.....", 5); // T
    structuredTetrominos[6] = Tetromino(L"..X...X..XX.....", 6); // Z

    for(int x=0; x<nFieldWidth ; x++ ){
        for(int y=0; y<nFieldHeight ; y++){
            if(x==0 || x== nFieldWidth-1 || y==nFieldHeight-1){
                pField[y][x]=9; //sets borders to 9
            }
            else{
                pField[y][x]=0; //sets inner area to 0
            }
        }
    }

    bool bKey[5];
    int nCurrentPiece=0;
    int nSpeed=20;
    int nSpeedCount=0;
    bool bForceDown=false;
    bool bRotateHold=true;
    bool bHardDropHold=true;
    int nPieceCount=0;
    int nScore=0;
    vector <int> vLines;
    bool bGameOver=false;


    
}
