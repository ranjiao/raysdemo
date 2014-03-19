#include <stdio.h>
#include <set>
#include <map>
#include <vector>
#include <assert.h>
#include <algorithm>

#define MAX_NODE_COUNT      32
#define INPUT_FILE          "map.in"
#define BIG_NUMBER          65535
#define MAX_PATH_NUM        128
#define K                   5

typedef std::set<int>               node_set;
typedef std::set<int>::iterator     note_set_iter;
typedef std::pair<int, int>         path_pair;              //路径长度和上一跳
typedef std::map<int, path_pair>    map_dist;
typedef map_dist::iterator          map_dist_iter;
typedef std::vector<int>            path_vec;
typedef path_vec::iterator          path_vec_iter;

int origin_map[MAX_NODE_COUNT * MAX_NODE_COUNT];            //原始邻接图
int crt_map1[MAX_NODE_COUNT * MAX_NODE_COUNT];              //去除了1条边以后的邻接图
int crt_map2[MAX_NODE_COUNT * MAX_NODE_COUNT];              //去除了2条边以后的邻接图

int start_node, end_node, node_count = MAX_NODE_COUNT;      //寻路的起点、终点和点的个数
node_set close_set, open_set;                               //dijkstra算法中的两个集合
map_dist dist2start;                                        //存储寻路以后每一个点到起点的距离和前驱

typedef std::vector< std::pair<int,path_vec> >  paths_vec;
typedef paths_vec::iterator                     paths_vec_iter;
paths_vec paths;       //所有找到的路径的长度和路径点

#define DIRECTIONAL_MAP

//读取输入文件
void ReadInput()
{
    FILE* infile = fopen(INPUT_FILE, "r");
    if(!infile)
    {
        printf("Error while open input file, exiting...\n");
        exit(1);
    }
    int cost = 0;

    fscanf(infile, "%d %d %d\n", &start_node, &end_node, &node_count);
    assert(start_node >= 0);
    assert(end_node <= node_count);

    for(int i=0; i<node_count; i++)
    {
        for(int j=0; j<node_count; j++)
        {
            fscanf(infile, "%d", &origin_map[i*node_count+j]);
            if(origin_map[i*node_count+j] == -1)
                origin_map[i*node_count+j] = BIG_NUMBER;
        }
        fscanf(infile, "\n");
    }
    memcpy(crt_map1, origin_map, sizeof(origin_map));
}

//将路径输出到屏幕
void PrintPath(path_vec path, int cost)
{
    printf("cost: %d, path: ", cost);
    for(path_vec_iter iter = path.begin();
        iter != path.end();
        iter++)
    {
        printf("=> %d ", *iter+1);
    }
    printf("\n");
}

//获得从x到y的距离（可能是删除了一条边以后的值）
int GetCrtMap(int x, int y)
{
#ifndef DIRECTIONAL_MAP
    if( x > y )
        return crt_map1[(y)*node_count+(x)];
    else
        return crt_map1[(x)*node_count+(y)];
#else
    if( x== y )
        return 0;
    return crt_map1[(x)*node_count+(y)];
#endif
}

//获得原始输入中x到y的距离
int GetOriginMap(int x, int y)
{
#ifndef DIRECTIONAL_MAP
    if( x > y )
        return origin_map[(y)*node_count+(x)];
    else
        return origin_map[(x)*node_count+(y)];
#else
    if( x== y )
        return 0;
    return origin_map[(x)*node_count+(y)];
#endif
}

//删除某一条边（将那条边的距离设置为很大）
void RemoveEdge(int x, int y, int* map = crt_map1)
{
#ifndef DIRECTIONAL_MAP
    if( x > y )
        crt_map1[y*node_count + x] = BIG_NUMBER;
    else
        crt_map1[x*node_count + y] = BIG_NUMBER;
#else
    crt_map1[x*node_count + y] = BIG_NUMBER;
#endif
}

//寻找最短路径，最短路径存储在path中。返回值为路径长度
int Dijkstra(path_vec& path)
{
    //初始化各个变量
    path.clear();
    dist2start.clear();
    open_set.clear();
    close_set.clear();

    close_set.insert(start_node);
    for(int i=0; i<node_count; i++)
    {
        if(i != start_node)
            open_set.insert(i);
        dist2start[i] = std::make_pair(start_node, GetCrtMap(start_node,i) );
    }

    //开始寻路
    while(!open_set.empty())
    {
        path_pair min_cost = std::make_pair(start_node, BIG_NUMBER);
        int min_cost_node = -1;
        for(note_set_iter nodeToDo = open_set.begin();
            nodeToDo != open_set.end();
            nodeToDo++)
        {
            for(note_set_iter nodeDone = close_set.begin();
                nodeDone != close_set.end();
                nodeDone++)
            {
                int tmp_cost = dist2start[*nodeDone].second + GetCrtMap(*nodeDone, *nodeToDo);
                if(min_cost.second >= tmp_cost)
                {
                    min_cost.first = *nodeDone;
                    min_cost.second = tmp_cost;
                    min_cost_node = *nodeToDo;
                }
            } 
        }
        assert(min_cost_node != start_node);
        if(min_cost_node < 0)
            return -1;
        open_set.erase(min_cost_node);
        close_set.insert(min_cost_node);
        dist2start[min_cost_node] = (min_cost);
        
        // update nodes connected with min_cost_node
        for(note_set_iter nodeRel = open_set.begin();
            nodeRel != open_set.end();
            nodeRel++)
        {
            int tmp_dist = GetCrtMap(min_cost_node, *nodeRel);
            if(tmp_dist < BIG_NUMBER && min_cost.second + tmp_dist < dist2start[*nodeRel].second)
            {
                dist2start[*nodeRel].first = min_cost_node;
                dist2start[*nodeRel].second = min_cost.second + tmp_dist < dist2start[*nodeRel].second;
            }
        }
    }

    //输出路径
    int crtNode = end_node;
    do
    {
        path.insert(path.begin(), crtNode);
        crtNode = dist2start[crtNode].first;
    }while( crtNode != start_node );
    path.insert(path.begin(), start_node);

    return dist2start[end_node].second;
}

// 对两条路径比较大小，最后对路径进行排序的时候用
bool Compare(std::pair<int, path_vec> a, std::pair<int, path_vec> b)
{
    return a.first > b.first;
}

void RemoveDunplicated(paths_vec& paths)
{
    for(paths_vec_iter i = paths.begin(); i != paths.end() && i+1 != paths.end(); i++)
    {
        paths_vec_iter j = i+1;
        while(j != paths.end())
        {
            if(i->first != j->first || i->second.size() != j->second.size())
            {
                j++;
                continue;
            }
            
            bool isEqual = true;
            for(unsigned int k=0; k<i->second.size(); k++)
                if(i->second[k] != j->second[k])
                {
                    isEqual = false;
                    break;
                }
            if(isEqual)
                j = paths.erase(j);
            else
                j++;
        }
    }
}

void main()
{
    ReadInput();
    path_vec shortestPath;                                      //最短路径

    int length = Dijkstra(shortestPath);
    paths.push_back(std::make_pair(length,shortestPath));       //找到最短路径

    //PrintPath(shortestPath, length);

    for(unsigned int i=0; i<shortestPath.size()-1; i++)
    {
        path_vec path1;                                         //去掉一条边以后的最短路径

        memcpy(crt_map1, origin_map, sizeof(origin_map));
        RemoveEdge(shortestPath[i], shortestPath[i+1]);         //删除一条边以后找出最短路径
        length = Dijkstra(path1);
        if(length==0)
            continue;
        
        paths.push_back(std::make_pair(length, path1));
        //PrintPath(path1, length);
        for(unsigned int j=0; j<path1.size()-1; j++)
        {
            path_vec path2;                                     //删除第二条边以后的最短路径
            memcpy(crt_map2, crt_map1, sizeof(crt_map1));
            RemoveEdge(path1[j], path1[j+1], crt_map2);
            length = Dijkstra(path2);

            if(length==0)
                continue;
            paths.push_back(std::make_pair(length, path2));
        }
    }

    //将找到的路径排序，然后输出(会存在路径长度相同但是路线不同的路径)
    std::sort(paths.begin(), paths.end(), Compare);

    //删除重复的路径
    RemoveDunplicated(paths);

    int outputCount = 0;
    for(paths_vec_iter iter = paths.begin();
        iter != paths.end();
        iter++)
    {
        if(outputCount++ > K)
            break;
        PrintPath(iter->second, iter->first);
    }
}