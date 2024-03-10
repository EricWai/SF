#include <curl/curl.h>
#include <bits/stdc++.h>
using namespace std;
#define map_dis 2
#define path_lock_num 5

// 定义坐标结构体
struct Point {
	int x, y;
	Point(int _x, int _y) : x(_x), y(_y) {}
    Point(){}
};
string url = "http://127.0.0.1:5555";
string url_create = url + "/create_submission";
string url_start = url + "/start";
string url_step = url + "/step";
string url_finish = url + "/finish_submission";
// 发送信息
string postParams = "";
// 接收信息
string response = "";
// 地图id
vector<string> maps(10, "");
int width, height;
enum type { MOVE, PICKUP, DELIVERY, STAY };
enum dir { LEFT, RIGHT, UP, DOWN, EMPTY };
// 定义两种类型四个方向的移动增量
vector<int> dir_left_x = { -1, 0, 1, 0 };
vector<int> dir_left_y = { 0, 1, 0, -1 };
vector<int> dir_up_x = { 0, 0, 1, -1 };
vector<int> dir_up_y = { -1, 1, 0, 0 };
// 搜索方向初始化
vector<int> dx = { 0, 0, 1, -1 };
vector<int> dy = { -1, 1, 0, 0 };
// 判断地图拥挤
bool crazy = false;
// 车辆优先级
vector<int> agv_priority;
//存放待处理的优先级交换
vector<vector<int>> priority_change; 
//所有货物位置(不在地上的是(-1,-1))
vector<Point> cargos_no;
//地图信息:-1:货物，架子，障碍物；0：可走空地；1：不可走空地；2：小车位置
vector<vector<int>> mapInfo;
//记录已完成货物数量
int cargos_done_num = 0;

class Agv;
class Shelve;
class Cargo;
vector<Agv*> agvs;
vector<Shelve*> shelves;
vector<Cargo*> cargos;

//函数声明
size_t req_reply(void* ptr, size_t size, size_t nmemb, void* stream);
void curl_post(string& _url, string& _postParams, string& _response);
dir SureDirection(int x, int y, int targetx, int targety);
vector<Point> BFS_empty(Point start, vector<Point> end, int agv_id);
vector<Point> BFS_full(Point start, vector<Point> end, int agv_id);
void ToJson(vector<string>& jsonstr, type t, dir d, Agv* agv);
void Clear();
void Start_BFS();
void Run(string mapId);
void SendJson(vector<string>& jsonstr, string map_id);


class Shelve {
public:
    int id;
    int cap;
    //payload为负载信息，-1表示空，序号为货物编号
    int payload;
    int x;
    int y;

};
class Cargo {
public:
    int id;
    //要去的货架编号
    int target; 
    int weight;
    int x;
    int y;
    //锁货物的agv_id，初始化-1
    int agv_id;
    //锁我的agv_dis，记得初始化无穷大
    int agv_dis;
    Cargo() {
        agv_dis = 100000;
        agv_id = -1;
    }
};
class Agv {
public:
    int id;
    //负载信息，-1表示空，正整数表示负载的货物编号
    int payload;
    //容量
    int cap;
    //车所在位置
    int x;
    int y;
    int cargo_id;
    int priority;
    int path_lock;
    //路线：从目标到车（不包括车）
    vector<Point> path; 
    Point pre;
    int empty_full;

    //想要前进移动
    void Move(vector<string>& jsonstr) {
        //path为空
        if (path.empty()) {
            ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
            return;
        }
        //path不为空
        else {
            //若下一格能走
            if (mapInfo[path.back().x][path.back().y] == 0) {
                //查询前进方向
                dir move_dir = SureDirection(x, y, path.back().x, path.back().y);
                ToJson(jsonstr, MOVE, move_dir, (Agv*)this);
                pre.x = x;
                pre.y = y;
                x = path.back().x;
                y = path.back().y;
                path.pop_back();
                mapInfo[x][y] = map_dis;
                mapInfo[pre.x][pre.y] = map_dis - 1;
            }
            //若下一格不能走
            else {
                ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
            }
        }
    }
    //碰撞查询函数。1：静止空车:相邻:有任务空车撞我；2：静止空:相邻:货车撞我
    //3：静止空车:隔一个:货车撞我；4：任务空车:相邻:与任务空车对撞
    //5：任务空车:相邻:撞静止空车；6：任务空车:相邻:与货车对撞
    //7：任务空车:隔一个:与货车对撞；8：货车:相邻:我撞静止空车
    //9：货车:相邻:与任务空车对撞；10：货车:判断下两个格子上是静止空车或者任务空车对撞
    //11：货车:相邻:货车撞我；12：静止货车:相邻:有一个货车撞我
    //13：货车:相邻:我撞了一个无目的货车
    pair<int, int> StrikeType() {
        int type = -1;
        int agv_id = -1;
        //空车
        if (payload == -1) {
            //静止空车
            if (path.size() == 0) {
                //1：静止空车:相邻:有任务空车撞我
                for (auto agv : agvs) {
                    if (!agv->path.empty() && agv->payload == -1 && agv->path.back().x == x && agv->path.back().y == y) {
                        type = 1;
                        agv_id = agv->id;
                        break;
                    }
                }
                //2：静止空车:相邻:货车撞我
                for (auto agv : agvs) {
                    if (!agv->path.empty() && agv->payload != -1 && agv->path.back().x == x && agv->path.back().y == y) {
                        type = 2;
                        agv_id = agv->id;
                        break;
                    }
                }
                //3：静止空车:隔一个:货车撞我
                for (auto agv : agvs) {
                    if (agv->path.size() >= 2 && agv->payload != -1 && agv->path[agv->path.size() - 2].x == x && agv->path[agv->path.size() - 2].y == y) {
                        type = 3;
                        agv_id = agv->id;
                        break;
                    }
                }
            }
            //任务车
            else if (path.size() >= 2) {
                //7：任务空车:隔一个:与货车对撞
                for (auto agv : agvs) {
                    if (agv->path.size() >= 2 && agv->payload != -1 && agv->path[agv->path.size() - 2].x == x && agv->path[agv->path.size() - 2].y == y) {
                        if (path.back().x == agv->path.back().x && path.back().y == agv->path.back().y) {
                            type = 7;
                            agv_id = agv->id;
                            break;
                        }
                    }
                }
                //4：任务空车:相邻:与任务空车对撞
                for (auto agv : agvs) {
                    if (!agv->path.empty() && agv->payload == -1 && agv->path.back().x == x && agv->path.back().y == y && path.back().x == agv->x && path.back().y == agv->y) {
                        type = 4;
                        agv_id = agv->id;
                        break;
                    }
                }
                //5：任务空车:相邻:撞静止空车
                for (auto agv : agvs) {
                    if (agv->path.empty() && agv->payload == -1 && path.back().x == agv->x && path.back().y == agv->y) {
                        type = 5;
                        agv_id = agv->id;
                        break;
                    }
                }
                //6：任务空车:相邻:与货车对撞
                for (auto agv : agvs) {
                    if (!agv->path.empty() && agv->payload != -1 && agv->path.back().x == x && agv->path.back().y == y && path.back().x == agv->x && path.back().y == agv->y) {
                        type = 6;
                        agv_id = agv->id;
                        break;
                    }
                }
            }
        }
        //货车
        else {
            //静止货车
            if (path.empty()) {
                //12：静止货车:相邻:有一个货车撞我
                for (auto agv : agvs) {
                    if (!agv->path.empty() && agv->payload != -1 && agv->path.back().x == x && agv->path.back().y == y) {
                        type = 12;
                        agv_id = agv->id;
                        break;
                    }
                }
            }
            //还有很多路的货车
            else if (path.size() >= 2) {
                //10：货车:判断下两个格子上是静止空车或者任务空车对撞
                for (auto agv : agvs) {
                    if (agv->payload == -1 && path[path.size() - 2].x == agv->x && path[path.size() - 2].y == agv->y) {
                        if (agv->path.empty() || (!agv->path.empty() && agv->path.back().x == path.back().x && agv->path.back().y == path.back().y)) {
                            type = 10;
                            agv_id = agv->id;
                            break;
                        }
                    }
                }
                //8：货车:相邻:我撞静止空车
                for (auto agv : agvs) {
                    if (agv->path.empty() && agv->payload == -1 && path.back().x == agv->x && path.back().y == agv->y) {
                        type = 8;
                        agv_id = agv->id;
                        break;
                    }
                }
                //9：货车:相邻:与任务空车对撞
                for (auto agv : agvs) {
                    if (!agv->path.empty() && agv->payload == -1 && agv->path.back().x == x && agv->path.back().y == y && path.back().x == agv->x && path.back().y == agv->y) {
                        type = 9;
                        agv_id = agv->id;
                        break;
                    }
                }
                //11：货车:相邻:货车对撞我
                for (auto agv : agvs) {
                    if (!agv->path.empty() && agv->payload != -1 && agv->path.back().x == x && agv->path.back().y == y && path.back().x == agv->x && path.back().y == agv->y) {
                        type = 11;
                        agv_id = agv->id;
                        break;
                    }
                }
                //13：货车:相邻:我撞了一个无目的货车
                for (auto agv : agvs) {
                    if (agv->path.empty() && agv->payload != -1 && path.back().x == agv->x && path.back().y == agv->y) {
                        type = 13;
                        agv_id = agv->id;
                        break;
                    }
                }
            }
        }
        return {type, agv_id};
    }
    //车行动的逻辑函数
    void action(vector<string>& jsonstr){
        //获取碰撞信息
        pair<int, int> strike_type = StrikeType();
        //状态车
        if (empty_full) {
            //货架方向
            dir shelf_dir = SureDirection(x, y, path.back().x, path.back().y);
            // 状态1：是空车，把错的货物拿起来
            if (empty_full == 1) {
                ToJson(jsonstr, PICKUP, shelf_dir, (Agv*)this);
                cargos_no[shelves[cargos[payload]->target]->payload].x = -1;
                cargos_no[shelves[cargos[payload]->target]->payload].y = -1;
                cargos[shelves[cargos[payload]->target]->payload]->agv_id = -1;
                cargos[shelves[cargos[payload]->target]->payload]->agv_dis = 100000;
                empty_full = 2;
            }
            // 状态2：把错误的货物放在空地上
            else if (empty_full == 2) {
                bool isFindEmpty = false;
                int nx, ny;
                for (int i = 0; i < 4; i++) {
                    nx = x + dx[i];
                    ny = y + dy[i];
                    if (nx >= 0 && nx < mapInfo.size() && ny >= 0 && ny < mapInfo[0].size() && mapInfo[nx][ny] == 0) {
                        isFindEmpty = true;
                        break;
                    }
                }
                //不能找到空地
                if (!isFindEmpty) {
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                }
                //能找到空地
                else {
                    dir empty_dir = SureDirection(x, y, nx, ny);
                    mapInfo[nx][ny] = -1;
                    cargos[shelves[cargos[payload]->target]->payload]->x = nx;
                    cargos[shelves[cargos[payload]->target]->payload]->y = ny;
                    cargos_no[shelves[cargos[payload]->target]->payload].x = nx;
                    cargos_no[shelves[cargos[payload]->target]->payload].y = ny;
                    cargos[shelves[cargos[payload]->target]->payload]->agv_id = -1;
                    cargos[shelves[cargos[payload]->target]->payload]->agv_dis = 100000;
                    empty_full = 3;
                    ToJson(jsonstr, DELIVERY, empty_dir, (Agv*)this);
                }
            }
            // 状态3：从背后拿起正确的货物
            else if (empty_full == 3) {
                dir cargo_dir = SureDirection(x, y, pre.x, pre.y);
                mapInfo[pre.x][pre.y] = map_dis - 1;
                empty_full = 4;
                ToJson(jsonstr, PICKUP, cargo_dir, (Agv*)this);
            }
            // 状态4：放正确的货在正确的货架
            else {
                shelves[cargos[payload]->target]->payload = payload;
                payload = -1;
                cargo_id = -1;
                path_lock = 0;
                path.clear();
                cargos_done_num++;
                empty_full = 0;
                ToJson(jsonstr, DELIVERY, shelf_dir, (Agv*)this);
            }
        }
        //空车
        else if (payload == -1) {
            //静止车
            if (path.empty()) {
                //1空空:相邻:有任务空车撞我
                if (strike_type.first == 1) {
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                }
                //2空货:相邻:货车撞我
                else if (strike_type.first == 2) {
                    bool isFindEmpty = false;
                    int nx, ny;
                    for (int i = 0; i < 4; i++) {
                        nx = x + dx[i];
                        ny = y + dy[i];
                        if (nx >= 0 && nx < mapInfo.size() && ny >= 0 && ny < mapInfo[0].size() && mapInfo[nx][ny] == 0) {
                            isFindEmpty = true;
                            break;
                        }
                    }
                    //如果周围有空
                    if (isFindEmpty) {
                        dir move_dir = SureDirection(x, y, nx, ny);
                        ToJson(jsonstr, MOVE, move_dir, (Agv*)this);
                        mapInfo[x][y] = map_dis - 1;
                        mapInfo[nx][ny] = map_dis;
                        pre.x = x;
                        pre.y = y;
                        x = nx;
                        y = ny;
                        path_lock = 0;
                        path.clear();
                    }
                    else {
                        ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    }
                }
                //3空货:隔一个:货车撞我
                else if (strike_type.first == 3) {
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    path_lock = 0;
                }
                else {
                    Move(jsonstr);
                }
            }
            //到达货物边上
            else if (path.size() == 1) {
                //若货物在理想位置开始PICKUP
                if (cargos[cargo_id]->x == path[0].x && cargos[cargo_id]->y == path[0].y) {
                    dir pickup_dir = SureDirection(x, y, path[0].x, path[0].y);
                    mapInfo[path[0].x][path[0].y] = map_dis - 1;
                    payload = cargo_id;
                    for(auto shelf : shelves){
                        if(shelf->x == path[0].x && shelf->y == path[0].y){
                            shelf->payload = -1;
                            mapInfo[path[0].x][path[0].y] = -1;
                        }
                    }
                    cargos[cargo_id]->x = -1;
                    cargos_no[cargo_id].x = -1;
                    cargos[cargo_id]->y = -1;
                    cargos_no[cargo_id].y = -1;
                    cargos[cargo_id]->agv_id = -1;
                    cargos[cargo_id]->agv_dis = 100000;
                    path.clear();
                    path_lock = 0;
                    //货车cargo_id是-1
                    cargo_id = -1;
                    ToJson(jsonstr, PICKUP, pickup_dir, (Agv*)this);
                }
                //若货物不见了
                else {
                    path.clear();
                    path_lock = 0;
                    cargo_id = -1;
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                }
            }
            //任务车//有很长的路
            else {
                //4空空:相邻:与任务空车对撞
                if (strike_type.first == 4) {
                    path_lock = 0;
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                }
                //5空空:相邻:撞静止空车
                else if (strike_type.first == 5) {
                    path_lock = 0;
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                }
                //6空货:相邻:与货车对撞
                else if (strike_type.first == 6) {
                    bool isFindEmpty = false;
                    int nx, ny;
                    for (int i = 0; i < 4; i++) {
                        nx = x + dx[i];
                        ny = y + dy[i];
                        if (nx >= 0 && nx < mapInfo.size() && ny >= 0 && ny < mapInfo[0].size() && mapInfo[nx][ny] == 0) {
                            isFindEmpty = true;
                            break;
                        }
                    }
                    //如果周围有空
                    if (isFindEmpty) {
                        dir move_dir = SureDirection(x, y, nx, ny);
                        ToJson(jsonstr, MOVE, move_dir, (Agv*)this);
                        mapInfo[x][y] = map_dis - 1;
                        mapInfo[nx][ny] = map_dis;
                        pre.x = x;
                        pre.y = y;
                        x = nx;
                        y = ny;
                        path_lock = 0;
                        path.push_back(Point(pre.x, pre.y));
                    }
                    else {
                        ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    }
                }
                //7空货:隔一个:与货车对撞
                else if (strike_type.first == 7) {
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    path_lock = 0;
                }
                //正常
                else {
                    Move(jsonstr);
                }
            }
        }

        //货车
        else {
            //没路径可走:无路去货架
            if (path.empty()) {
                if (strike_type.first == 12) {
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                }
                else {
                    ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                }
            }
            //与货架相邻
            else if (path.size() == 1) {
                //查询货架和我的位置关系
                dir shelf_dir = SureDirection(x, y, path.back().x, path.back().y);
                // 货架为空（正常车放货）
                if (shelves[cargos[payload]->target]->payload == -1) {
                    shelves[cargos[payload]->target]->payload = payload;
                    cargos_done_num++;
                    payload = -1;
                    cargo_id = -1;
                    path.clear();
                    path_lock = 0;
                    ToJson(jsonstr, DELIVERY, shelf_dir, (Agv*)this);
                }
                // 货架不为空（准备进入状态机）
                else{
                    //后面能放
                    if (mapInfo[pre.x][pre.y] == 0) {
                        dir empty_dir = SureDirection(x, y, pre.x, pre.y);
                        mapInfo[pre.x][pre.y] = -1;
                        empty_full = 1;
                        cargo_id = -1;
                        path_lock = 0;
                        ToJson(jsonstr, DELIVERY, empty_dir, (Agv*)this);
                    }
                    else {
                        ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    }  
                }
            }
            //有很长的路
            else {
                //8我是货车:相邻:我撞静止空车
                if (strike_type.first == 8) {
                    bool isFindEmpty = false;
                    int nx, ny;
                    for (int i = 0; i < 4; i++) {
                        nx = x + dx[i];
                        ny = y + dy[i];
                        if (nx >= 0 && nx < mapInfo.size() && ny >= 0 && ny < mapInfo[0].size() && mapInfo[nx][ny] == 0) {
                            isFindEmpty = true;
                            break;
                        }
                    }
                    //周围有空地
                    if (isFindEmpty) {
                        dir move_dir = SureDirection(x, y, nx, ny);
                        ToJson(jsonstr, MOVE, move_dir, (Agv*)this);
                        mapInfo[x][y] = map_dis - 1;
                        mapInfo[nx][ny] = map_dis;
                        pre.x = x;
                        pre.y = y;
                        x = nx;
                        y = ny;
                        path.push_back(Point(pre.x, pre.y));
                    }
                    else {
                        ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    }
                }
                //9空货:相邻:与任务空车对撞
                else if (strike_type.first == 9) {
                    bool isFindEmpty = false;
                    int nx, ny;
                    for (int i = 0; i < 4; i++) {
                        nx = x + dx[i];
                        ny = y + dy[i];
                        if (nx >= 0 && nx < mapInfo.size() && ny >= 0 && ny < mapInfo[0].size() && mapInfo[nx][ny] == 0) {
                            isFindEmpty = true;
                            break;
                        }
                    }
                    //周围有空地
                    if (isFindEmpty) {
                        dir move_dir = SureDirection(x, y, nx, ny);
                        ToJson(jsonstr, MOVE, move_dir, (Agv*)this);
                        mapInfo[x][y] = map_dis - 1;
                        mapInfo[nx][ny] = map_dis;
                        pre.x = x;
                        pre.y = y;
                        x = nx;
                        y = ny;
                        path.push_back(Point(pre.x, pre.y));
                    }
                    else {
                        ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    }
                }
                //10空货:隔一个:判断是撞了静止空车或者任务空车对撞 
                else if (strike_type.first == 10) {
                    dir next_dir = SureDirection(x, y, path.back().x, path.back().y);
                    // 前面的那个格子能放  
                    if (mapInfo[path.back().x][path.back().y] == 0) {
                        ToJson(jsonstr, DELIVERY, next_dir, (Agv*)this);
                        mapInfo[path.back().x][path.back().y] = -1;
                        cargos_no[payload].x = path.back().x;
                        cargos_no[payload].y = path.back().y;
                        cargos[payload]->x = path.back().x;
                        cargos[payload]->y = path.back().y;
                        cargos[payload]->agv_id = -1;
                        cargos[payload]->agv_dis = 100000;
                        path_lock = 0;
                        
                        cargo_id = -1;
                        payload = -1;
                        // 对方的优先级更低
                        if (agvs[strike_type.second]->priority > priority) {
                            priority_change.push_back({strike_type.second, id }) ;
                        }
                    }
                    else {
                        ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    }
                }
                //11货货:相邻:货车对撞我
                else if (strike_type.first == 11) {
                    // 我的优先级高
                    if (agvs[strike_type.second]->priority > priority) {
                        vector<Point> targetShelf;
                        targetShelf.push_back(Point(shelves[cargos[payload]->target]->x, shelves[cargos[payload]->target]->y));
                        mapInfo[agvs[strike_type.second]->x][agvs[strike_type.second]->y] = -1;
                        path = BFS_full(Point(x, y), targetShelf, id);
                        // 没搜到
                        if (path.empty()) {
                            ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                        }
                        // 搜到, 上锁，挪一步
                        else {
                            path_lock = path_lock_num;
                            Move(jsonstr);
                        }
                        mapInfo[agvs[strike_type.second]->x][agvs[strike_type.second]->y] = map_dis;

                    }
                    else {
                        vector<Point> targetShelf;
                        targetShelf.push_back(Point(shelves[cargos[payload]->target]->x, shelves[cargos[payload]->target]->y));
                        mapInfo[agvs[strike_type.second]->x][agvs[strike_type.second]->y] = -1;
                        path = BFS_full(Point(x, y),targetShelf,id);
                        // 没搜到
                        if (path.empty()) {
                            ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                        }
                        // 搜到, 上锁，挪一步
                        else {
                            path_lock = path_lock_num;
                            Move(jsonstr);
                        }
                        mapInfo[agvs[strike_type.second]->x][agvs[strike_type.second]->y] = map_dis;
                    }
                }
                //13货货:相邻:我撞了一个无目的货车
                else if (strike_type.first == 13) {
                    vector<Point> targetShelf;
                    targetShelf.push_back(Point(shelves[cargos[payload]->target]->x, shelves[cargos[payload]->target]->y));
                    mapInfo[agvs[strike_type.second]->x][agvs[strike_type.second]->y] = -1;
                    path = BFS_full(Point(x, y), targetShelf, id);
                    // 如果没搜到
                    if (path.empty()) {
                        ToJson(jsonstr, STAY, EMPTY, (Agv*)this);
                    }
                    // 如果搜到, 上锁，挪一步
                    else {
                        path_lock = path_lock_num;
                        Move(jsonstr);
                    }
                    mapInfo[agvs[strike_type.second]->x][agvs[strike_type.second]->y] = map_dis;
                }
                else {
                    Move(jsonstr);
                }
            }
        }
    }
    
    Agv() {
        path.clear();
        cargo_id = -1;
        path_lock = 0;
        pre.x = -1;
        pre.y = -1;
        empty_full = 0;
    }
};


// 确定行动方向
dir SureDirection(int x, int y, int targetx, int targety) {
    int dxx = targetx - x;
    int dyy = targety - y;
    dir d;
    if (dxx == 0 && dyy == -1) d = UP;
    else if (dxx == 0 && dyy == 1) d = DOWN;
    else if (dxx == 1 && dyy == 0) d = RIGHT;
    else d = LEFT;
    return d;
}
// 空车BFS
vector<Point> BFS_empty(Point start, vector<Point> end, int agv_id) {
    int which_cargos = -1;

    // 定义棋盘大小
    const int ROWS = mapInfo.size();
    const int COLS = mapInfo[0].size();

    // 记录起点到每一格的距离
    vector<vector<int>> temp_dis(ROWS, vector<int>(COLS, 0));

    // 定义棋盘上的障碍物位置
    vector<Point> obstacles;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (mapInfo[i][j] == -1) {
                class Point cur(i, j);
                obstacles.emplace_back(cur);
            }
        }
    }

    // 定义二维数组来标记是否访问过某个点
    vector<vector<bool>> visited(ROWS, vector<bool>(COLS, false));

    // 定义队列用于广度优先搜索
    queue<Point> q;
    q.push(start);
    visited[start.x][start.y] = true;

    // 定义二维数组来存储点的前驱坐标，用于还原路径
    vector<vector<Point>> prev(ROWS, vector<Point>(COLS, Point(-1, -1)));

    // 广度优先搜索
    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        bool found = false;

        
        for (int i = 0; i < end.size(); i++) {
            // 找到任意的物体
            if (curr.x == end[i].x && curr.y == end[i].y) {
                //能抢
                if (temp_dis[end[i].x][end[i].y] < cargos[i]->agv_dis || agvs[agv_id]->cargo_id == i) {
                    //改车
                    agvs[agv_id]->cargo_id = i;
                    //改货
                    cargos[i]->agv_id = agv_id;
                    cargos[i]->agv_dis = temp_dis[end[i].x][end[i].y];
                    found = true;
                    which_cargos = i;
                    break;
                }
            }
        }
        if (found)
            break;
        // 在四个方向上进行搜索
        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            // 检查新点是否有效且未访问过，并且不是障碍物
            if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS && !visited[nx][ny]) {
                bool isObstacle = false;
                for (const auto& obstacle : obstacles) {
                    if (obstacle.x == nx && obstacle.y == ny) {
                        isObstacle = true;
                        break;
                    }
                }
                bool isEnd = false;
                for (const auto& ending : end) {
                    if (ending.x == nx && ending.y == ny) {
                        isEnd = true;
                        break;
                    }
                }
                //非障碍物
                if (!isObstacle) {
                    temp_dis[nx][ny] = temp_dis[curr.x][curr.y] + 1;
                    q.push(Point(nx, ny));
                    visited[nx][ny] = true;
                    prev[nx][ny] = curr;
                }
                //是货物
                else if (isEnd && isObstacle) {
                    temp_dis[nx][ny] = temp_dis[curr.x][curr.y] + 1;
                    q.push(Point(nx, ny));
                    visited[nx][ny] = true;
                    prev[nx][ny] = curr;
                }
            }
        }
    }
    vector<Point> path;
    path.clear();
    //未找到
    if (which_cargos == -1) {
        return path;
    }
    //找到
    Point curr = end[which_cargos];
    while (!(curr.x == start.x && curr.y == start.y)) {
        Point Bcurr(curr.x, curr.y);
        path.push_back(Bcurr);
        curr = prev[curr.x][curr.y];
    }
    return path;
}
// 货车BFS
vector<Point> BFS_full(Point start, vector<Point> end, int agv_id) {
    int which_cargos = -1;

    // 定义棋盘大小
    const int ROWS = mapInfo.size();
    const int COLS = mapInfo[0].size();

    // 定义棋盘上的障碍物位置
    vector<Point> obstacles;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (mapInfo[i][j] == -1) {
                class Point cur(i, j);
                obstacles.emplace_back(cur);
            }
        }
    }

    // 定义二维数组来标记是否访问过某个点
    vector<vector<bool>> visited(ROWS, vector<bool>(COLS, false));

    // 定义队列用于广度优先搜索
    queue<Point> q;
    q.push(start);
    visited[start.x][start.y] = true;

    // 定义二维数组来存储点的前驱坐标，用于还原路径
    vector<vector<Point>> prev(ROWS, vector<Point>(COLS, Point(-1, -1)));

    // 广度优先搜索
    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        bool found = false;

        // 找到对应货架，退出搜索
        for (int i = 0; i < end.size(); i++) {
            if (curr.x == end[i].x && curr.y == end[i].y) {
                found = true;
                which_cargos = i;
            }
        }

        if (found)
            break;


        // 在四个方向上进行搜索
        for (int i = 0; i < 4; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            // 检查新点是否有效且未访问过，并且不是障碍物
            if (nx >= 0 && nx < ROWS && ny >= 0 && ny < COLS && !visited[nx][ny]) {
                bool isObstacle = false;
                for (const auto& obstacle : obstacles) {
                    if (obstacle.x == nx && obstacle.y == ny) {
                        isObstacle = true;
                        break;
                    }
                }
                bool isEnd = false;
                for (const auto& ending : end) {
                    if (ending.x == nx && ending.y == ny) {
                        isEnd = true;
                        break;
                    }
                }
                //非障碍物
                if (!isObstacle) {
                    q.push(Point(nx, ny));
                    visited[nx][ny] = true;
                    prev[nx][ny] = curr;
                }
                //是货架
                else if (isEnd && isObstacle) {
                    q.push(Point(nx, ny));
                    visited[nx][ny] = true;
                    prev[nx][ny] = curr;
                    break;
                }
            }
        }
    }
    vector<Point> path;
    path.clear();
    //未找到
    if (which_cargos == -1) {
        return path;
    }
    //找到
    Point curr = end[which_cargos];
    while (!(curr.x == start.x && curr.y == start.y)) {
        Point Bcurr(curr.x, curr.y);
        path.push_back(Bcurr);
        curr = prev[curr.x][curr.y];
    }
    return path;
}
// 动作转JSON
void ToJson(vector<string>& jsonstr, type t, dir d, Agv* agv) {
    string str = "";
    if (t == STAY) {
        str = "\"type\": \"STAY\"";
    }
    else if (t == MOVE) {
        if (d == LEFT) str = "\"type\":\"MOVE\",\"dir\" : \"LEFT\"";
        else if (d == RIGHT) str = "\"type\":\"MOVE\",\"dir\" : \"RIGHT\"";
        else if (d == UP) str = "\"type\":\"MOVE\",\"dir\" : \"UP\"";
        else str = "\"type\":\"MOVE\",\"dir\" : \"DOWN\"";
    }
    else if (t == PICKUP) {
        if (d == LEFT) str = "\"type\":\"PICKUP\",\"dir\" : \"LEFT\"";
        else if (d == RIGHT) str = "\"type\":\"PICKUP\",\"dir\" : \"RIGHT\"";
        else if (d == UP) str = "\"type\":\"PICKUP\",\"dir\" : \"UP\"";
        else str = "\"type\":\"PICKUP\",\"dir\" : \"DOWN\"";
    }
    else {
        if (d == LEFT) str = "\"type\":\"DELIVERY\",\"dir\" : \"LEFT\"";
        else if (d == RIGHT) str = "\"type\":\"DELIVERY\",\"dir\" : \"RIGHT\"";
        else if (d == UP) str = "\"type\":\"DELIVERY\",\"dir\" : \"UP\"";
        else str = "\"type\":\"DELIVERY\",\"dir\" : \"DOWN\"";
    }
    str = "{" + str + "}";
    jsonstr[agv->id] = str;
}
// 行动过后地图结算
void Clear() {
	//交换优先级
    for (int i = 0; i < priority_change.size();i++) {
        swap(agvs[priority_change[i][0]]->priority, agvs[priority_change[i][1]]->priority);
        swap(agv_priority[agvs[priority_change[i][0]]->priority], agv_priority[agvs[priority_change[i][1]]->priority]);
    }
    priority_change.clear();
    //清除地图上的数字
    for (int i = 0; i < mapInfo.size(); i++) {
        for (int j = 0; j < mapInfo[0].size(); j++) {
            if (mapInfo[i][j] >= 1 && mapInfo[i][j] < map_dis) {
                mapInfo[i][j]--;
            }
        }
    }
    return;
}
// 任务部署
void Start_BFS() {
    //所有车按照优先级遍历，安排任务
    for (int k = 0; k < agv_priority.size(); k++) {
        // 状态车
        if (agvs[agv_priority[k]]->empty_full != 0) continue;
        // 有锁车
        if (agvs[agv_priority[k]]->path_lock != 0) {
            agvs[agv_priority[k]]->path_lock--;
            continue;
        }
        // 起点坐标
        Point start(agvs[agv_priority[k]]->x, agvs[agv_priority[k]]->y);
        // 若是空车
        if (agvs[agv_priority[k]]->payload == -1) {
            // 路线：从货物到车（不包括车）
            agvs[agv_priority[k]]->path = BFS_empty(start, cargos_no, agvs[agv_priority[k]]->id);
        }
        // 若是货车
        else{ 
            vector<Point> jiazi;
            jiazi.push_back(Point(shelves[cargos[agvs[agv_priority[k]]->payload]->target]->x, shelves[cargos[agvs[agv_priority[k]]->payload]->target]->y));
            // 路线：从架子到车（不包括车）
            agvs[agv_priority[k]]->path = BFS_full(start, jiazi, agvs[agv_priority[k]]->id);
        }
        // 更新锁信息
        for (auto cargo : cargos) {
            if (cargo->agv_id != -1 && agvs[cargo->agv_id]->cargo_id != cargo->id) {
                cargo->agv_id = -1;
                cargo->agv_dis = 100000;
            }
        }
        for (auto agv : agvs) {
            if (agv->cargo_id != -1 && cargos[agv->cargo_id]->agv_id != agv->id) {
                agv->path.clear();
                agv->path_lock = 0;
                agv->cargo_id = -1;
            }
        }
    }
    // 更新锁信息(总体)
    for (auto cargo : cargos) {
        if (cargo->agv_id != -1 && agvs[cargo->agv_id]->cargo_id != cargo->id) {
            cargo->agv_id = -1;
            cargo->agv_dis = 100000;
        }
    }
    for (auto agv : agvs) {
        if (agv->cargo_id != -1 && cargos[agv->cargo_id]->agv_id != agv->id) {
            agv->path.clear();
            agv->path_lock = 0;
            agv->cargo_id = -1;
        }
    }
}
// 运行函数
void Run(string mapId) {
    int time = 1;
    while (time++) {
        if (time >= 300) {
            break;
        }
        vector<string> jsonstr(agvs.size());
		Start_BFS();
        //判断是否拥挤(crazy)
        int agv_begin_id = agvs.size() - cargos.size()/agvs.size();
        int time_lock = agvs.size()/2;
        if(time <= time_lock && crazy){
            for(int i = agv_begin_id;i < agvs.size();i++){
                agvs[i]->path.clear();
            }
        }
        //所有车行动
        for (int k = 0; k < agv_priority.size(); k++) {
            agvs[agv_priority[k]]->action(jsonstr);
        }
		Clear();
        SendJson(jsonstr, mapId);
        //已完成任务
        if (cargos_done_num == cargos.size()) {
			break;
        }
	}
	return;
}
// 接收返回信息流
size_t req_reply(void* ptr, size_t size, size_t nmemb, void* stream){
    string* str = (string*)stream;
    (*str).append((char*)ptr, size * nmemb);
    return size * nmemb;
}
// post通信
void curl_post(string& _url, string& _postParams, string& _response) {
    CURL* curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        //添加head，代表数据为json格式
        curl_slist* plist = curl_slist_append(NULL, "Content-Type:application/json;");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
        //发送的url
        curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
        //待发送的数据
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, _postParams.c_str());
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        //接受数据
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&_response);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            url = "http://sf-judge-server:5555";
        }
        else {
            //cout << response << endl;//输出数据
        }
        curl_easy_cleanup(curl);
    }
    return;
}
// 找到下标i后第一个数字
int findFirstNum(const string& s, int i) {
    while (i < s.size()) {
        if (s[i] >= '0' && s[i] <= '9') {
            break;
        }
        i++;
    }
    int j = i;
    string str;
    while (j < s.size()) {
        if (s[j] < '0' || s[j] > '9') {
            j--;
            break;
        }
        else {
            str += s[j];
            j++;
        }
    }
    if (str.empty()) return 0;
    return stoi(str);
}
// agv信息
void findAgvs(const string& jsonstr, int& front, int& late) {
    int start = jsonstr.find("{", front);
    int end = jsonstr.find("}", front);

    while (end < late) { 
        Agv* agv = new Agv(); 
        int i = jsonstr.find("id", start);
        agv->id = findFirstNum(jsonstr, i);
        agv->priority = agv->id;
        i = jsonstr.find("payload", start);
        i += 9; 
        if (jsonstr[i] == 'n' || jsonstr[i] == 'N') {
            agv->payload = -1;
        }
        else {
            int j = jsonstr.find(",", i);
            string str = jsonstr.substr(i, j - i);
            agv->payload = stoi(str);
        }
        i = jsonstr.find("cap", start);
        agv->cap = findFirstNum(jsonstr, i);
        agvs.emplace_back(agv);
        start = jsonstr.find("{", start + 1);
        end = jsonstr.find("}", end + 1);
    }
}
// cargo信息
void findCargos(const string& jsonstr, int& front, int& late) {
    int start = jsonstr.find("{", front);
    int end = jsonstr.find("}", front);
    while (end < late) {
        Cargo* cargo = new Cargo();
        int i = jsonstr.find("id", start);
        cargo->id = findFirstNum(jsonstr, i);
        i = jsonstr.find("target", start);
        cargo->target = findFirstNum(jsonstr, i);
        i = jsonstr.find("weight", start);
        cargo->weight = findFirstNum(jsonstr, i);
        cargos.emplace_back(cargo);
        start = jsonstr.find("{", start + 1);
        end = jsonstr.find("}", end + 1);
    }
}
// shelves信息
void findShelves(const string& jsonstr, int& front, int& late) {
    int start = jsonstr.find("{", front);
    int end = jsonstr.find("}", front);
    while (end < late) {
        Shelve* shelve = new Shelve();
        int i = jsonstr.find("id", start);
        shelve->id = findFirstNum(jsonstr, i);
        i = jsonstr.find("cap", start);
        shelve->cap = findFirstNum(jsonstr, i);
        i = jsonstr.find("payload", start);
        i += 9;
        if (jsonstr[i] == 'n' || jsonstr[i] == 'N') {
            shelve->payload = -1;
        }
        else {
            int j = jsonstr.find("}", i);
            shelve->payload = stoi(jsonstr.substr(i, j - i));
        }
        shelves.emplace_back(shelve);
        start = jsonstr.find("{", start + 1);
        end = jsonstr.find("}", end + 1);
    }
}
// 地图障碍信息
void findMap(vector<vector<int>>& mapInfo, const string& jsonstr, int& front, int& late) {
    int start = jsonstr.find("{", front);
    int end = jsonstr.find("}", front);
    while (end < late) {
        int i;
        i = jsonstr.find("type", start);
        i += 7;
        if (jsonstr[i] == 'a') {
            i = jsonstr.find("id", start);
            int id = findFirstNum(jsonstr, i);
            i = jsonstr.find("x", start);
            int x = findFirstNum(jsonstr, i);
            i = jsonstr.find("y", start);
            int y = findFirstNum(jsonstr, i);
            agvs[id]->x = x;
            agvs[id]->y = y;
            mapInfo[x][y] = map_dis;
        }
        else if (jsonstr[i] == 'c') {
            i = jsonstr.find("id", start);
            int id = findFirstNum(jsonstr, i);
            i = jsonstr.find("x", start);
            int x = findFirstNum(jsonstr, i);
            i = jsonstr.find("y", start);
            int y = findFirstNum(jsonstr, i);
            cargos[id]->x = x;
            cargos[id]->y = y;
            mapInfo[x][y] = -1;
        }
        else if (jsonstr[i] == 's') {
            i = jsonstr.find("id", start);
            int id = findFirstNum(jsonstr, i);
            i = jsonstr.find("x", start);
            int x = findFirstNum(jsonstr, i);
            i = jsonstr.find("y", start);
            int y = findFirstNum(jsonstr, i);
            shelves[id]->x = x;
            shelves[id]->y = y;
            mapInfo[x][y] = -1;
        }
        else {
            i = jsonstr.find("x", start);
            int x = findFirstNum(jsonstr, i);
            i = jsonstr.find("y", start);
            int y = findFirstNum(jsonstr, i);
            mapInfo[x][y] = -1;
        }

        start = jsonstr.find("{", start + 1);
        end = jsonstr.find("}", end + 1);
    }
}
// 解析json
vector<vector<int>> ParingJson(const string& jsonstr) {
    int front = 0, late = 0;
    int width, height;
    front = jsonstr.find("width", front);
    width = findFirstNum(jsonstr, front);
    front = jsonstr.find("height", front);
    height = findFirstNum(jsonstr, front);
    int index = jsonstr.find("map_state");
    front = jsonstr.find("agvs", index);
    if (front != -1) {
        front = jsonstr.find("[", front); 
        late = jsonstr.find("]", front);
        findAgvs(jsonstr, front, late);
    }
    front = jsonstr.find("cargos", index);
    if (front != -1) {
        front = jsonstr.find("[", front);
        late = jsonstr.find("]", front);
        findCargos(jsonstr, front, late);
    }
    front = jsonstr.find("shelves", index);
    if (front != -1) {
        front = jsonstr.find("[", front);
        late = jsonstr.find("]", front);
        findShelves(jsonstr, front, late);
    }
    vector<vector<int>> mapInfo(width, vector<int>(height, 0));
    front = jsonstr.find("map", front);
    front = jsonstr.find("[", front);
    late = jsonstr.find("]", front);
    findMap(mapInfo, jsonstr, front, late);
    return mapInfo;
}
// 将每一步的指令发送出去
void SendJson(vector<string>& jsonstr, string map_id) { 
    string sendMsg = "{\"actions\": [";
    string tail = "], \"map_id\": \"" + map_id + "\"}";
    for (auto& str : jsonstr) {
        sendMsg += str + ",";
    }
    sendMsg.pop_back();
    sendMsg += tail;
    string resp = "";
    curl_post(url_step, sendMsg, resp);
}

int main() {

    //url_test
    postParams = "";
    response = "";
    curl_post(url_create, postParams, response);
    //更新url
    url_create = url + "/create_submission";
    url_start = url + "/start";
    url_step = url + "/step";
    url_finish = url + "/finish_submission";
    //create
    postParams = "";
    response = "";
    curl_post(url_create, postParams, response);
    cout << response << endl;
    //读取map名字
    for (int i = 0; i < 10; i++) {
        maps[i].push_back(response[30 + 5 * i]);
        maps[i].push_back(response[31 + 5 * i]);
        cout << maps[i] << " ";
    }
    cout << endl;
    //start，依次进行第k张地图
    for (int k = 0; k < maps.size(); k++) {
        postParams = "{\"map_id\":\"" + maps[k] + "\"}";
        response = "";
        curl_post(url_start, postParams, response);
        cout << response << endl;
        //输出地图初始状态数据
        cout << maps[k] << " " << endl;
        //解析json并初始化地图
        agvs.clear();
        cargos.clear();
        shelves.clear();
        mapInfo = ParingJson(response);
        crazy = false;
        cargos_done_num = 0;
        // 拥挤率
        float crowding_rate = (float)((float)agvs.size() * (float)cargos.size() - (float)mapInfo.size() * (float)mapInfo[0].size()) / (float)((float)mapInfo.size() * (float)mapInfo[0].size());
        // 空地率
        float space_rate = (float)((float)mapInfo.size() * (float)mapInfo[0].size()) / (float)((float)mapInfo.size() * (float)mapInfo[0].size() - (float)agvs.size() - (float)cargos.size() - (float)shelves.size());
        // 是否拥挤
        float crazy_level = crowding_rate - space_rate;
        if (crazy_level > 0) {
            crazy = true;
        }
        // 选择搜索方向
        int direct_dir = 0;
        for (auto cargo : cargos) {
            direct_dir += abs(cargo->x - shelves[cargo->target]->x) - abs(cargo->y - shelves[cargo->target]->y);
        }
        if (direct_dir > 0) {
            dx = dir_left_x;
            dy = dir_left_y;
        }
        else{
            dx = dir_up_x;
            dy = dir_up_y;
        }
        // 放置在货架里的货物初始化坐标
        for (int i = 0; i < shelves.size(); i++) {
            if (shelves[i]->payload != -1) {
                cargos[shelves[i]->payload]->x = shelves[i]->x;
                cargos[shelves[i]->payload]->y = shelves[i]->y;
                dx = dir_up_x;
                dy = dir_up_y;
            }
        }
        // 初始化未拿的货物
        cargos_no.clear();
        cargos_no.resize(cargos.size());
        for (int i = 0; i < cargos.size(); i++) {
            // 货物在目标货架
            if (shelves[cargos[i]->target]->x == cargos[i]->x && shelves[cargos[i]->target]->y == cargos[i]->y) {
                cargos_done_num++;
                cargos_no[i] = (Point(-1, -1));
            }
            else {
                cargos_no[i] = (Point(cargos[i]->x, cargos[i]->y));
            }
        }
        agv_priority.resize(agvs.size());
        for (int i = 0; i < agvs.size(); i++) {
            agv_priority[i] = i;
        }
        Run(maps[k]);
    }
    // 结束
    postParams = "";
    response = "";
    curl_post(url_finish, postParams, response);
	return 0;
}