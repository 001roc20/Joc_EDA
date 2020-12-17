#include "Player.hh"
#include "Structs.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME HomelanderV3


struct PLAYER_NAME : public Player {

    /**
    * Factory: returns a new instance of this class.
    * Do not modify this function.
    */
    static Player* factory () {
        return new PLAYER_NAME;
    }
    
    /**
    * Types and attributes for your player can be defined here.
    */
    typedef vector<bool> row;
    typedef vector<row> matrix;

    vector<int> b;
    vector<int> w;
    
    const vector<Dir> dirs = {Up,Down,Left,Right};
    set<int> buildersInABarricade;
    vector<bool> builtABarricade;

    int num_barricades = 0;

    bool hasBetterWeapon(WeaponType myWeap, WeaponType enemyWeap) {
        if(myWeap == Bazooka or (myWeap == Gun and enemyWeap != Bazooka) or (myWeap == Hammer and enemyWeap == Hammer))
            return false;
        return true;
    }

    bool hasMoreHealth(int myId, int enemyId) {
        return citizen(myId).life < citizen(enemyId).life;
    }
    
    Dir oppositeDir(Dir d) {
        if(d == Up)
            return Down;
        else if(d == Down)
            return Up;
        else if(d == Left)
            return Right;
        else
            return Left;
    }


    bool isThereAnEnemy(Pos p, CitizenType c) {
        int cellId = cell(p).id;
        if(cellId != -1 and citizen(cellId).player != me() and citizen(cellId).type == c)
            return true;
        return false;
    }
    
    struct str {
        Pos p;
        int dist;
        Dir* firstMove;
    };

    int bfsBonus(Pos p, BonusType b, Dir& firstMoveReturn) {
        matrix m(board_rows(), row(board_cols(), false));
        str i; i.p = p, i.dist = 0, i.firstMove = NULL;
        m[i.p.i][i.p.j] = true;
        queue<str> q;
        q.push(i);
        while(not q.empty()) {
            i = q.front();
            q.pop();
            for(Dir d : dirs) {
                if(pos_ok(i.p+d)) {
                    if(not m[(i.p+d).i][(i.p+d).j]) {
                        if(cell(i.p+d).bonus == b) {
                            if(i.firstMove == NULL)
                                firstMoveReturn = d;
                            else
                                firstMoveReturn = *i.firstMove;
                            return i.dist;
                        }
                        else if(cell(i.p+d).type != Building) {
                            if(not is_day() or (is_day() and (cell(i.p+d).b_owner == -1 or cell(i.p+d).b_owner == me()))) { 
                                str j; j.p = i.p+d, j.dist = i.dist+1; j.firstMove = new Dir;
                                if(i.firstMove == NULL)
                                    *j.firstMove = d;
                                else 
                                    *j.firstMove = *i.firstMove;
                                q.push(j);
                            }
                        }
                        m[(i.p+d).i][(i.p+d).j]=true;
                    }
                }
            }
        }
        return -1;
    }

    int bfsWarrior(Pos p, Dir& firstMoveReturn, int id, int dist) {
       matrix m(board_rows(), row(board_cols(), false));
        str i; i.p = p, i.dist = 0, i.firstMove = NULL;
        m[i.p.i][i.p.j] = true;
        queue<str> q;
        q.push(i);
        while(not q.empty()) {
            i = q.front();
            q.pop();
            
            if(dist != -1 and i.dist > dist)
                return -1;
            
            for(Dir d : dirs) {
                if(pos_ok(i.p+d)) {
                    if(not m[(i.p+d).i][(i.p+d).j]) {
                        if(isThereAnEnemy(i.p+d,Warrior)) {
                            if(i.firstMove == NULL)
                                firstMoveReturn = d;
                            else
                                firstMoveReturn = *i.firstMove;
                            return i.dist;
                        }
                        else if(cell(i.p+d).type != Building and cell(i.p+d).id == -1) {
                            str j; j.p = i.p+d, j.dist = i.dist+1; j.firstMove = new Dir;
                            if(i.firstMove == NULL)
                                *j.firstMove = d;
                            else 
                                *j.firstMove = *i.firstMove;
                            q.push(j);
                            m[j.p.i][j.p.j] = true;
                        }
                        m[(i.p+d).i][(i.p+d).j]=true;
                    }
                }
            }
        }
        return -1;
    }

    int bfsBarricades(Pos p, Dir& firstMoveReturn) {
        matrix m(board_rows(), row(board_cols(), false));
        str i; i.p = p, i.dist = 0, i.firstMove = NULL;
        m[i.p.i][i.p.j] = true;
        queue<str> q;
        q.push(i);
        while(not q.empty()) {
            i = q.front();
            q.pop();

            for(Dir d : dirs) {
                if(pos_ok(i.p+d)) {
                    if(not m[(i.p+d).i][(i.p+d).j]) {
                        if(cell(i.p+d).b_owner == me() and cell(i.p+d).id == -1) {
                            if(i.firstMove == NULL)
                                firstMoveReturn = d;
                            else
                                firstMoveReturn = *i.firstMove;
                            return i.dist;
                        }
                        else if(cell(i.p+d).type != Building and cell(i.p+d).b_owner == -1 and cell(i.p+d).id == -1) {
                            str j; j.p = i.p+d, j.dist = i.dist+1; j.firstMove = new Dir;
                            if(i.firstMove == NULL)
                                *j.firstMove = d;
                            else 
                                *j.firstMove = *i.firstMove;
                            q.push(j);
                        }
                        m[(i.p+d).i][(i.p+d).j]=true;
                    }
                }
            }
        }
        return -1;
    }

    int bfsWeapon(Pos p, Dir& firstMoveReturn) {
        matrix m(board_rows(), row(board_cols(), false));
        str i; i.p = p, i.dist = 0, i.firstMove = NULL;
        m[i.p.i][i.p.j] = true;
        queue<str> q;
        q.push(i);
        while(not q.empty()) {
            i = q.front();
            q.pop();
            
            for(Dir d : dirs) {
                if(pos_ok(i.p+d)) {
                    if(not m[(i.p+d).i][(i.p+d).j]) {
                        if(cell(i.p+d).weapon == Gun or cell(i.p+d).weapon == Bazooka) {
                            if(i.firstMove == NULL)
                                firstMoveReturn = d;
                            else
                                firstMoveReturn = *i.firstMove;
                            return i.dist;
                        }
                        else if(cell(i.p+d).type != Building) {
                            if(cell(i.p+d).type != Building and (cell(i.p+d).b_owner == -1 or cell(i.p+d).b_owner == me())) { 
                                str j; j.p = i.p+d, j.dist = i.dist+1; j.firstMove = new Dir;
                                if(i.firstMove == NULL)
                                    *j.firstMove = d;
                                else 
                                    *j.firstMove = *i.firstMove;
                                q.push(j);
                            }
                        }
                        m[(i.p+d).i][(i.p+d).j]=true;
                    }
                }
            }
        }
        return -1;
    }

    int bfsCitizen(Pos p, Dir& firstMoveReturn, int id) {
        matrix m(board_rows(), row(board_cols(), false));
        str i; i.p = p, i.dist = 0, i.firstMove = NULL;
        m[i.p.i][i.p.j] = true;
        queue<str> q;
        q.push(i);
        while(not q.empty()) {
            i = q.front();
            q.pop();

            for(Dir d : dirs) {
                if(pos_ok(i.p+d)) {
                    if(not m[(i.p+d).i][(i.p+d).j]) {
                        int enemyId = cell(i.p+d).id;
                        if(isThereAnEnemy(i.p+d,Builder) or (isThereAnEnemy(i.p+d,Warrior) and not hasBetterWeapon(citizen(id).weapon,citizen(cell(i.p+d).id).weapon))) {
                            if(i.firstMove == NULL)
                                firstMoveReturn = d;
                            else
                                firstMoveReturn = *i.firstMove;
                            return i.dist;
                        }
                        else if(cell(i.p+d).type != Building and cell(i.p+d).id == -1) {
                            str j; j.p = i.p+d, j.dist = i.dist+1; j.firstMove = new Dir;
                            if(i.firstMove == NULL)
                                *j.firstMove = d;
                            else 
                                *j.firstMove = *i.firstMove;
                            q.push(j);
                        }
                        m[(i.p+d).i][(i.p+d).j]=true;
                    }
                }
            }
        }
        return -1;
    }

    int bfsBuilder(Pos p, Dir& firstMoveReturn, int id) {
        matrix m(board_rows(), row(board_cols(), false));
        str i; i.p = p, i.dist = 0, i.firstMove = NULL;
        m[i.p.i][i.p.j] = true;
        queue<str> q;
        q.push(i);
        while(not q.empty()) {
            i = q.front();
            q.pop();
            
            for(Dir d : dirs) {
                if(pos_ok(i.p+d)) {
                    if(not m[(i.p+d).i][(i.p+d).j]) {
                        int enemyId = cell(i.p+d).id;
                        if(isThereAnEnemy(i.p+d, Builder)) {
                            if(i.firstMove == NULL)
                                firstMoveReturn = d;
                            else
                                firstMoveReturn = *i.firstMove;
                            return i.dist;
                        }
                        else if(cell(i.p+d).type != Building and (not is_day() or (is_day() and (cell(i.p+d).b_owner == -1 or cell(i.p+d).b_owner==me()))) and cell(i.p+d).id == -1) {
                            str j; j.p = i.p+d, j.dist = i.dist+1; j.firstMove = new Dir;
                            if(i.firstMove == NULL)
                                *j.firstMove = d;
                            else 
                                *j.firstMove = *i.firstMove;
                            q.push(j);
                        }
                        m[(i.p+d).i][(i.p+d).j]=true;
                    }
                }
            }
        }
        return -1;
    }

    bool allValuesUsed(vector<bool>& v) {
        for(int i = 0; i < int(v.size()); ++i)
            if(not v[i])
                return false;
        return true;
    }

    bool buildBarricade(int id) {
        Pos a = citizen(id).pos;
        int i = 0;
        bool built = false;
        while(not built and i < 4) {
            if(pos_ok(a+dirs[i])) {
                Pos nextPos = a+dirs[i];
                if((cell(nextPos).is_empty() and num_barricades < max_num_barricades()) or (cell(nextPos).b_owner == me() and cell(nextPos).resistance != barricade_max_resistance())) {
                    build(id,dirs[i]);
                    ++num_barricades;
                    built = true;
                }
            }
            ++i;
        }
        return built;
    }

    void moveRandomly(int id) {
        Pos p = citizen(id).pos;
        vector<bool> v(4, false);
        int i = random(0,3);
        bool a = allValuesUsed(v);
        while(not a) {
            v[i] = true;
            if(pos_ok(p+dirs[i]) and cell(p+dirs[i]).is_empty()) {
                move(id,dirs[i]);
                return;
            }
            a = allValuesUsed(v);
            i = random(0,3);
        }
    }
    
    bool isPosToEscape(Pos p) {
        if(pos_ok(p) and cell(p).type != Building and ((cell(p).id == -1 and cell(p).b_owner == me()) or cell(p).b_owner == -1))
            return true;
        return false;
    }

    //p must be a position with an enemy warrior

    /**
    * Play method, invoked once per each round.
    */
    virtual void play () {
        b = builders(me());
        w = warriors(me());
        
        if(is_day()) {
            buildersInABarricade.clear();
            
            for(int id : w) {
                Pos a = citizen(id).pos;
                Dir d;
                if(citizen(id).life > 0) {
                    if(citizen(id).life < warrior_ini_life()/2 and bfsBonus(a,Food,d) != -1)
                        move(id,d);
                    else if(citizen(id).weapon != Bazooka and bfsWeapon(a,d) != -1)
                        move(id,d);
                    else if(bfsBuilder(a,d,id) != -1)
                        move(id,d);
                    else if(bfsBonus(a,Money,d) != -1)
                        move(id,d);
                    else
                        moveRandomly(id);
                }
            }
            
            for(int id : b) {
                Pos a = citizen(id).pos;
                Dir d;
                if(citizen(id).life > 0 and not buildBarricade(id)) {
                    if(citizen(id).life < builder_ini_life()/2 and bfsBonus(a,Food,d) != -1)
                        move(id,d);
                    else if(bfsBonus(a,Money,d) != -1)
                        move(id,d);
                    else
                        moveRandomly(id);
                }
            }
        }
        else {
            num_barricades = 0;
            
            for(int id : w) {
                Pos a = citizen(id).pos;
                Dir d;
                if(citizen(id).life > 0) {
                    if(citizen(id).life <= warrior_ini_life()/4 and bfsBonus(a,Food,d) != -1) 
                        move(id,d);
                    else if(citizen(id).weapon != Hammer and citizen(id).life > warrior_ini_life()/4 and bfsCitizen(a,d,id) != -1)
                        move(id,d);
                    else if(bfsBuilder(a,d,id) != -1)
                        move(id,d);
                    else
                        moveRandomly(id);
                }
            }
            
            for(int id : b) {
                Pos a = citizen(id).pos;
                Dir d;
                if(citizen(id).life > 0) {
                    if(cell(a).b_owner==me()) {
                        buildersInABarricade.insert(id);
                        if(bfsWarrior(a,d,id,1) != -1) {
                            if(isPosToEscape(a+oppositeDir(d)))
                                move(id,oppositeDir(d));
                            else {
                                bool moved = false;
                                for(Dir d2 : dirs) {
                                    if(d2 != d and d2 != oppositeDir(d) and not moved) {
                                        if(isPosToEscape(a+d2)) {
                                            move(id,d2);
                                            moved=true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else {
                        buildersInABarricade.erase(id);
                        if(bfsWarrior(a,d,id,4) != -1) {
                            if(isPosToEscape(a+oppositeDir(d)))
                                move(id,oppositeDir(d));
                            else {
                                bool moved = false;
                                for(Dir d2 : dirs) {
                                    if(d2 != d and d2 != oppositeDir(d) and not moved) {
                                        if(isPosToEscape(a+d2)) {
                                            move(id,d2);
                                            moved=true;
                                        }
                                    }
                                }
                            }
                        }
                        else if(buildersInABarricade.size() < barricades(me()).size() and bfsBarricades(a,d) != -1)
                            move(id,d);
                        else
                            moveRandomly(id);
                    }
                }
            }
        }
    }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
