#include "Player.hh"
#include "Structs.hh"

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME HomelanderV6


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

    int num_barricades = 0;

    bool hasBetterWeapon(WeaponType myWeap, WeaponType enemyWeap) {
        if(myWeap == Bazooka or (myWeap == Gun and enemyWeap != Bazooka) or (myWeap == Hammer and enemyWeap == Hammer))
            return false;
        return true;
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
        int dir;
    };
    
    bool conditions(Pos newPos, Pos p, const int flag) {
        switch(flag) {
            case 1:
                return cell(newPos).bonus == Money;
            case 2:
                return cell(newPos).bonus == Food;
            case 3:
                return isThereAnEnemy(newPos,Builder);
            case 4:
                return (cell(newPos).weapon == Gun and hasBetterWeapon(citizen(cell(p).id).weapon,Gun)) or
                                    (cell(newPos).weapon == Bazooka and hasBetterWeapon(citizen(cell(p).id).weapon,Bazooka));
            case 5:
                return isThereAnEnemy(newPos,Warrior) or isThereAnEnemy(newPos,Builder);
            case 6:
                return cell(newPos).b_owner == me() and cell(newPos).id == -1;
            case 7:
                return isThereAnEnemy(newPos,Warrior);
            case 8:
                return isThereAnEnemy(newPos,Warrior);
        }
        return false;
    }
    
    bool moveConditions(Pos newPos, const int flag) {
        switch(flag) {
            case 1:
                return cell(newPos).type != Building and (cell(newPos).b_owner == me() or cell(newPos).b_owner == -1) 
                and cell(newPos).id == -1;
            case 2:
                return cell(newPos).type != Building and (cell(newPos).b_owner == me() or cell(newPos).b_owner == -1)
                and cell(newPos).id == -1;
            case 3:
                return cell(newPos).type != Building and (cell(newPos).b_owner == me() or cell(newPos).b_owner == -1)
                and cell(newPos).id == -1;
            case 4:
                return cell(newPos).type != Building and (cell(newPos).b_owner == me() or cell(newPos).b_owner == -1)
                and cell(newPos).id == -1;
            case 5:
                return cell(newPos).type != Building and cell(newPos).id == -1;
            case 6:
                return cell(newPos).type != Building and (cell(newPos).b_owner == me() or cell(newPos).b_owner == -1) 
                and cell(newPos).id == -1;
            case 7:
                return cell(newPos).type != Building;
            case 8:
                return cell(newPos).type != Building;
        }
        return false;
    }
    
    bool bfs(Pos p, int& ret, const int flag) {
        matrix visited(board_rows(), row(board_cols(), false));
        str node; node.p = p, node.dist = 0, node.dir = -1;
        visited[node.p.i][node.p.j] = true;
        queue<str> q; q.push(node);
        while(not  q.empty()) {
            node = q.front();
            q.pop();
            
            switch(flag) {
                case 7:
                    if(node.dist > 1) {
                        return false;
                    }
                case 8:
                    if(node.dist > 4) {
                        return false;
                    }
            }
            
            for(int i = 0; i < 4; ++i) {
                Pos newPos = node.p+dirs[i];
                if(pos_ok(newPos)) {
                    if(not visited[newPos.i][newPos.j]) {
                        if(conditions(newPos, p, flag)) {
                            if(node.dir == -1) {
                                ret = i;
                            }
                            else {
                                ret = node.dir;
                            }
                            return true;
                        }
                        else if(moveConditions(newPos,flag)) {
                            str newNode; newNode.dist = node.dist+1, newNode.p = newPos;
                            if(node.dir == -1) {
                                newNode.dir = i;
                            }
                            else {
                                newNode.dir = node.dir;
                            }
                            q.push(newNode);
                        }
                    }
                    visited[newPos.i][newPos.j] = true;
                }
            }
        }
        return false;
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

    bool escape(Pos a) {
        int ret, id = cell(a).id;
        if(bfs(a,ret,8)) {
            if(isPosToEscape(a+oppositeDir(dirs[ret]))) {
                move(id,oppositeDir(dirs[ret]));
                return true;
            }
            else {
                bool moved = false;
                for(Dir d : dirs) {
                    if(d != dirs[ret] and d != oppositeDir(dirs[ret])) {
                        if(isPosToEscape(a+d)) {
                            move(id,d);
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

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
                if(citizen(id).life > 0) {
                    int ret;
                    if(citizen(id).life < warrior_ini_life()/2 and bfs(a,ret,2)) {
                        move(id,dirs[ret]);
                    }
                    else if(citizen(id).weapon != Bazooka and bfs(a,ret,4))  {
                        move(id,dirs[ret]);
                    }
                    else if(bfs(a,ret,3)) {
                        move(id,dirs[ret]);
                    }
                    else if(bfs(a,ret,1)) {
                        move(id,dirs[ret]);
                    }
                }
            }
            
            for(int id : b) {
                Pos a = citizen(id).pos;
                if(citizen(id).life > 0) {
                    int ret;
                    if(not buildBarricade(id)) {
                        if(citizen(id).life < builder_ini_life()/2 and bfs(a,ret,2)) {
                            move(id,dirs[ret]);
                        }
                        else if(bfs(a,ret,1))  {
                            move(id,dirs[ret]);
                        }
                        else {
                            moveRandomly(id);
                        }
                    }
                }
            }
        }
        else {
            num_barricades = 0;
            
            for(int id : w) {
                Pos a = citizen(id).pos;
                if(citizen(id).life > 0) {
                    int ret;
                    if(citizen(id).life < warrior_ini_life()/4 and bfs(a,ret,2)) {
                        move(id,dirs[ret]);
                    }
                    else if(citizen(id).weapon != Hammer and citizen(id).life >= warrior_ini_life()/4 and bfs(a,ret,5))  {
                        move(id,dirs[ret]);
                    }
                    else if(bfs(a,ret,3)) {
                        move(id,dirs[ret]);
                    }
                    else if(bfs(a,ret,4)) {
                        move(id,dirs[ret]);
                    }
                    else {
                        moveRandomly(id);
                    }
                }
            }
            
            for(int id : b) {
                Pos a = citizen(id).pos;
                Dir d;
                if(citizen(id).life > 0) {
                    int ret;
                    if(cell(a).b_owner != me()) {
                        buildersInABarricade.erase(id);
                        if(not escape(a)) {
                            if(int(buildersInABarricade.size()) < max_num_barricades() and bfs(a,ret,6)) {
                                move(id,dirs[ret]);
                            }
                            else if(bfs(a,ret,1)) {
                                move(id,dirs[ret]);
                            }
                            else if(bfs(a,ret,2)) {
                                move(id,dirs[ret]);
                            }
                            else {
                                moveRandomly(id);
                            }
                        }
                    }
                    else {
                        buildersInABarricade.insert(id);
                        if(cell(a).resistance < barricade_max_resistance()/3 and bfs(a,ret,7) and isPosToEscape(a+oppositeDir(dirs[ret]))) {
                            move(id,oppositeDir(dirs[ret]));
                        }
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
