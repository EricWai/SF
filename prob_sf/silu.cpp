while(steps){

    start_bfs(函数)
        {
            if（path_lock）
                continue;
            else{
                if(empty_full)  continue;

                if（空车）//拿货
                {
                    path=BFS_empty();   //path.size（）带处理（0，非0）

                }
                else (非空) //送货
                {
                    
                    path = BFS_full();

                }
            }
            for (auto cargo : cargos) //我的车被抢了//目的修改//货//前任agv_id agv_dis；
            for (auto agv : agvs)     //更新被抢的人
        }
    }
    for(按优先级每个车agv行动){
        agv.action();
    }

BFS_empty(){
    //搜地图
    if(搜到了){
        //判断是不是目前锁定this
        //判断能不抢走
        if(能){
            //抢走
            //车//agv->cargo_id
                //path_lock不变
                //结束返回值修改path
            //货//现任agv_id agv_dis；
        }
        else{
            //还需搜
        }
    }
}

BFS_full(){
    if(找到路){
        //返回路
    }
    else 空
}

action(){
    //状态车
    if(empty_full){
        //是空车，想把错的拿起来
        if(empty_full == 1){
            //方向json(PICKUP)
            empty_full = 2
        }
        //把错误的货物放在空地上
        else if(empty_full == 2){
            if(有空地){
                //方向json(DELIVERY)  (agv,shelf,cargo,mapinfo,nums)                //？？？？货车贴贴，无地可放
                //地图上要出现那个错误的货物（货架上）
                empty_full = 3
            }
            //无空地
            else {
                //json(STAY)
            }
        }
        //从背后拿起正确的货物
        else if(empty_full == 3){
            //方向json(PICKUP) (agv,shelf,cargo,mapinfo,nums) 
            //障碍物清除
            empty_full = 4
        }
        //放正确的货在正确的货架
        else{
            //方向json(DELIVERY)  (agv,shelf,cargo,mapinfo,nums)（巨多）
            empty_full = 0
        }
    }
    
    //空车
    else if(空车){
        //按path.size()分类
        //没路径可走:1.地图地上没货了
        //(静止车)
        if(path.size() == 0){
            if(1空空:相邻:有任务空车撞我){
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
            }
            else if(2空货:相邻:货车撞我){
                //搜三格move，搜不到stay
            }
            else if(3空货:隔一个:货车撞我){
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                //准备拿货，马上我就优先于他了
            }
            else{
                move();
            }
        }
        //与货物相邻
        else if(path.size() == 1){
            if(是PICKUP){
                //方向json(PICKUP)  (agv,shelf,cargo,mapinfo,nums)
            }
            //不能PICKUP
            else{
                //json(STAY)  (agv,shelf,cargo,mapinfo,nums)
            }
        }
        
        //还有很多路
        //任务车
        else{
            if(4空空:相邻:与任务空车对撞){
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)//换任务//可冷静
            }
            else if(5空空:相邻:撞静止空车){
                //解锁
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
            }
            else if(6空货:相邻:与货车对撞){
                //搜三格
                if(搜成功了){
                    //方向json(MOVE)  (agv,shelf,cargo,mapinfo,nums)
                }
                else{
                    //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                }
            }
            else if(7空货:隔一个:与货车对撞){
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                //准备拿货，马上我就优先于他了
            }
            else{
                move();
            }
        }
    }
    
    //货车
    //交换货物来改优先级
    else{
        //按path.size()分类
        //没路径可走:无路去货架
        if(path.size() == 0){
            if(12货货:相邻:有一个货车撞我){
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
            }
            else {
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
            }
        }
        //与货架相邻
        else if(path.size() == 1){
            if(货架不为空){
                if(能DELIVERY){
                    //empty_full = 1;
                    //方向json(DELIVERY)  (agv,shelf,cargo,mapinfo,nums)
                    //放下货物设为障碍物（不能被偷）把自己的cargo_id = -1
                }else{
                    //json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                } 
            }
            //货架为空
            else{
                //方向json(DELIVERY)  (agv,shelf,cargo,mapinfo,nums)
            }
        }
        
        //还有很多路
        else{
            if(8空货:相邻:我撞静止空车){
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                //我走，搜三格//path补一格
            }
            else if(9空货:相邻:与任务空车对撞){
                //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                //我走，搜三格//path补一格
            }
            else if(10空货:隔一个:(bool)search_next判断是静止空车或者任务空车){
                //可能任务车是lock
                if(能放) {
                     //方向json(DELIVERY)  (agv,shelf,cargo,mapinfo,nums)//将对方优先级设定比我高级
                }
                else {
                    //  stay
                }
               
            }
            else if(11货货:相邻:货车对撞我){
                if(优先级高){
                    //将对方设置为障碍物，BFS 
                    if(path.szie() == 0){
                        //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                    }
                    else{
                        //lock
                        //方向json(MOVE)  (agv,shelf,cargo,mapinfo,nums)
                    }
                }
                else{
                    //将对方设置为障碍物，BFS
                    if(path.szie() == 0){
                        //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                    }
                    else{
                        //lock
                        //方向json(MOVE)  (agv,shelf,cargo,mapinfo,nums)
                    }
                }
            }
            else if(13货货:相邻:我撞了一个无目的货车){
                //lock
                //方向json(MOVE)  (agv,shelf,cargo,mapinfo,nums)
            }
            else if(14货货:相邻:我撞了一个有目的货车（他不撞我）){
                //将对方设置为障碍物，BFS
                if(path.szie() == 0){
                    //方向json(STAY)  (agv,shelf,cargo,mapinfo,nums)
                }
                else{
                    //lock
                    //方向json(MOVE)  (agv,shelf,cargo,mapinfo,nums)
                }
            }
            else{
                move();
            }
        }
    }
}

//strike_type//返回碰撞类型//记得可以调各种情况的位置（多种情况满足）
//加大两个距离
//创建一个agv内类函数用来扫描下两个格子上是否有无任务空车或者与我对撞空车（货车调用）（(bool)search_next）
//目的交换货物


/**疯子贯穿全局**/
/**优先级越小越牛逼**/
/*碰撞
    空空
        相邻
            有任务空车对撞
            有任务空车撞静止空车
        隔一个
            不处理（下一步会转化为相邻）
    空货
        相邻
            货车撞静止空车（尽力避免）
            货车对撞任务空车（空车视货车为障碍物搜路）//path_lock//go home
        隔一个
            货车path的第二个为静止空车或者对撞空车
    货货
        相邻
            2有目的货车 货货对撞（优先级低的掉头）搜不到交换优先级
            有一个每目的 货货对撞（优先级低的掉头）搜不到交换优先级
            2个有目的的货车，不对撞，单向撞（肇事者设障碍物重搜，被撞的人不管）
        隔一个
            不处理
*/   
// 定义一个全局变量
// DX DY

/*
main()
    mapInfo;
    全部更新完毕（cargos）(货架上的)
    定义   左或者上 （两个局部变量）
    int direct_dir = 0;
    for(cargos){
        //direct_dir += abs(cargos.x-cargos货架.x)-abs(cargos.y-cargos货架.y);
    }
    if(direct_dir < 0){
        //上
    }else{
        //左
    }
    
*/

// 定义四个方向的移动增量 左下右上 除了前三个图最快
// const int dx[] = { -1, 0, 1, 0 };
// const int dy[] = { 0, 1, 0, -1 };
//971 ac10 上下右左 前三最快
// const int dx[] = { 0, 0, 1, -1 };
// const int dy[] = { -1, 1, 0, 0 };
/*不碰

*/
