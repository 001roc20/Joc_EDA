#include <unistd.h>
#include <wait.h>
#include <string>
#include <time.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <vector>
#include <cstdlib>

void usage() {
    std::cout << "Usage: ./test player1 player2 player3 player4 n flag" << std::endl;
    std::cout << "Flag: r to play with random seeds, f to play with 1 to the n seeds" << std::endl;
    std::cout << "Default input file: default.cnf" << std::endl;
    std::cout << "Default output file: default.res" << std::endl;
}

int main(int argv, char* argc[]) {
    if(argv != 7) {
        usage();
        exit(1);
    }
    
    char flag=*argc[6];
    if(flag!='r' and flag!='f') {
        std::cout << "Flag must be f for fixed seeds or r for random seeds" << std::endl;
        exit(1);
    }
    
    int maxPoints=-1;
    char* players[4]={argc[1], argc[2], argc[3], argc[4]};
    std::vector<int> wins(4,0);
    std::vector<int> score(4);
    
    //Seed for random numbers, based on current time
    srand((unsigned int)time(NULL));
    int seed=0;
    
    for(int i = 0; i < atoi(argc[5])*4; ++i) {
        //Genrate a new random game seed every 4 iterations
        if(i%4==0) {
            if(flag=='r')
                seed = rand()%10000;
            else
                ++seed;
        }
        
        int ret = fork();
        //Child plays the game
        if(ret==0) {
            //Close stantard input
            close(0);
            //default.cnf becomes the stantard input
            int fd=open("default.cnf",O_RDONLY);
            if(fd==-1) {
                std::cout << "Unable to open default.cnf configuration file" << std::endl;
                exit(1);
            }
            //Open default.res
            fd=open("default.res",O_WRONLY);
            if(fd==-1) {
                std::cout << "Unable to open default.res output file" << std::endl;
                exit(1);
            }
            //Close stantard output
            close(1);
            //Close cerr output
            close(2);
            //Duplicate default.res output to set it as stantard output
            dup2(fd,1);
            char randChar[4];
            sprintf(randChar,"%d",seed);
            //Execute the Game
            execlp("./Game", "Game", "-s", randChar, players[i%4], players[(i+1)%4], players[(i+2)%4], players[(i+3)%4], NULL);
        }
        //Parent waits
        else {
            int status;
            //Wait the child
            waitpid(ret,&status,0);
            if(WIFEXITED(status)) {
                //Exit if child exits with exit(1)
                if(WEXITSTATUS(status)) {
                    exit(1);
                }
            }
            else {
                //Exit if child exits because of an assertion
                if(WTERMSIG(status)==6) {
                    std::cout << "Assertion triggered" << std::endl;
                    std::cout << "Probably a player name is wrong" << std::endl;
                    std::cout << "Seed: " << seed << std::endl;
                    exit(1);
                }
            }
            
            //Get total number of rounds
            int days, rxday;
            std::ifstream file;
            std::string aux, finalRound;
            file.open("default.res");
            while(file >> aux and aux != "NUM_DAYS");
            file >> days;
            while(file >> aux and aux != "NUM_ROUNDS_PER_DAY");
            file >> rxday;
            finalRound = "round " + std::to_string(rxday*days);
            
            //Get final score
            while(std::getline(file,aux) and aux != finalRound);
            while(file >> aux and aux != "score");
            file >> score[0] >> score[1] >> score[2] >> score[3];
            int k=0, maxScore=-1;
            for(int j = 0; j < 4; ++j) {
                if(maxScore < score[j]) {
                    maxScore=score[j];
                    k=j;
                }
            }
            maxPoints=std::max(maxScore,maxPoints);
            ++wins[(i+k)%4];
            file.close();
            
            std::system("clear");
            std::cout << i+1 << " games played" << std::endl;
            std::cout << "Seed: " << seed << std::endl;
            for(int j = 0; j < 4; ++j) {
                int percentage=(wins[j]/double(i+1))*100;
                std::cout << argc[j+1] << " wins " << wins[j] << " times" << std::endl;
                std::cout << "[";
                for(int l = 0; l < percentage; ++l)
                    std::cout << "#";
                for(int l = 0; l < 100-percentage; ++l)
                    std::cout << " ";
                std::cout << "]" << std::endl;
            }
        }
    }
    
    std::system("clear");
    std::cout << argc[5] << "*4 GAMES PLAYED" << std::endl;
    for(int j = 0; j < 4; ++j) {
        int percentage=wins[j]/double(atoi(argc[5])*4)*100;
        std::cout << argc[j+1] << " wins " << wins[j] << " times" << std::endl;
        std::cout << "[";
        for(int i = 0; i < percentage; ++i)
            std::cout << "#";
        for(int i = 0; i < 100-percentage; ++i)
            std::cout << " ";
        std::cout << "]" << std::endl;
    }
    std::cout << "Maximum score: " << maxPoints << std::endl;
}
