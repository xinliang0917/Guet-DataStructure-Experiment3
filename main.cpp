#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stack>

// 交通方式枚举
enum TransportMode {
    ROAD,
    RAILWAY,
    AIR
};

// 前驱信息结构体
typedef struct PreInfo {
    int preVex;
    enum TransportMode transportMode;
} PreInfo;

// 图的邻接矩阵表示
typedef struct GraphMatrix {
    int size;                    // 图中节点的个数
    char** city;                 // 城市名称数组，下标为节点编号
    int** graphRoad;             // 公路费用矩阵
    int** graphRailway;          // 铁路费用矩阵
    int** graphAir;              // 航空费用矩阵
    int** timeRoad;              // 公路时间矩阵
    int** timeRailway;           // 铁路时间矩阵
    int** timeAir;               // 航空时间矩阵
} GraphMatrix;

// 搜索维度枚举
enum SearchDimension {
    BY_COST,
    BY_TIME
};

// 函数原型声明
int findCityIndex(GraphMatrix* graph, const char* cityName);
int addCity(GraphMatrix* graph, const char* cityName);
GraphMatrix* createGraph();
void destroyGraph(GraphMatrix* graph);
GraphMatrix* readTransportData(const char* filename);
void dijkstra(GraphMatrix* graph, int start, int* dist, PreInfo* pre, 
             int allowRoad, int allowRailway, int allowAir, enum SearchDimension dimension);
const char* getTransportModeString(enum TransportMode mode);
void printPath(GraphMatrix* graph, int start, int end, int* dist, PreInfo* pre, enum SearchDimension dimension);
void showCities(GraphMatrix* graph);
void addConnection(GraphMatrix* graph, const char* fromCity, const char* toCity, enum TransportMode mode, int cost, int time);
void removeConnection(GraphMatrix* graph, const char* fromCity, const char* toCity, enum TransportMode mode);

// 查找城市在数组中的索引
int findCityIndex(GraphMatrix* graph, const char* cityName) {
    for (int i = 0; i < graph->size; i++) {
        if (strcmp(graph->city[i], cityName) == 0) {
            return i;
        }
    }
    return -1;
}

// 添加城市到图中
int addCity(GraphMatrix* graph, const char* cityName) {
    int index = findCityIndex(graph, cityName);
    if (index != -1) {
        return index; // 城市已存在
    }
    
    // 扩展城市数组
    char** newCityArray = (char**)realloc(graph->city, (graph->size + 1) * sizeof(char*));
    if (!newCityArray) {
        return -1;
    }
    graph->city = newCityArray;
    
    // 分配城市名称内存并复制
    graph->city[graph->size] = (char*)malloc(strlen(cityName) + 1);
    if (!graph->city[graph->size]) {
        return -1;
    }
    strcpy(graph->city[graph->size], cityName);
    
    // 扩展邻接矩阵
    int newSize = graph->size + 1;
    
    // 公路矩阵
    int** newRoadMatrix = (int**)realloc(graph->graphRoad, newSize * sizeof(int*));
    if (!newRoadMatrix) {
        return -1;
    }
    graph->graphRoad = newRoadMatrix;
    graph->graphRoad[graph->size] = (int*)malloc(newSize * sizeof(int));
    
    // 铁路矩阵
    int** newRailwayMatrix = (int**)realloc(graph->graphRailway, newSize * sizeof(int*));
    if (!newRailwayMatrix) {
        return -1;
    }
    graph->graphRailway = newRailwayMatrix;
    graph->graphRailway[graph->size] = (int*)malloc(newSize * sizeof(int));
    
    // 航空矩阵
    int** newAirMatrix = (int**)realloc(graph->graphAir, newSize * sizeof(int*));
    if (!newAirMatrix) {
        return -1;
    }
    graph->graphAir = newAirMatrix;
    graph->graphAir[graph->size] = (int*)malloc(newSize * sizeof(int));
    
    // 公路时间矩阵
    int** newTimeRoadMatrix = (int**)realloc(graph->timeRoad, newSize * sizeof(int*));
    if (!newTimeRoadMatrix) {
        return -1;
    }
    graph->timeRoad = newTimeRoadMatrix;
    graph->timeRoad[graph->size] = (int*)malloc(newSize * sizeof(int));
    
    // 铁路时间矩阵
    int** newTimeRailwayMatrix = (int**)realloc(graph->timeRailway, newSize * sizeof(int*));
    if (!newTimeRailwayMatrix) {
        return -1;
    }
    graph->timeRailway = newTimeRailwayMatrix;
    graph->timeRailway[graph->size] = (int*)malloc(newSize * sizeof(int));
    
    // 航空时间矩阵
    int** newTimeAirMatrix = (int**)realloc(graph->timeAir, newSize * sizeof(int*));
    if (!newTimeAirMatrix) {
        return -1;
    }
    graph->timeAir = newTimeAirMatrix;
    graph->timeAir[graph->size] = (int*)malloc(newSize * sizeof(int));
    
    // 初始化新增的行列，设置为无穷大（用-1表示）
    for (int i = 0; i < newSize; i++) {
        graph->graphRoad[i][graph->size] = -1;
        graph->graphRailway[i][graph->size] = -1;
        graph->graphAir[i][graph->size] = -1;
        graph->timeRoad[i][graph->size] = -1;
        graph->timeRailway[i][graph->size] = -1;
        graph->timeAir[i][graph->size] = -1;
        
        if (i < graph->size) {
            graph->graphRoad[graph->size][i] = -1;
            graph->graphRailway[graph->size][i] = -1;
            graph->graphAir[graph->size][i] = -1;
            graph->timeRoad[graph->size][i] = -1;
            graph->timeRailway[graph->size][i] = -1;
            graph->timeAir[graph->size][i] = -1;
        }
    }
    
    int result = graph->size;
    graph->size++;
    return result;
}

// 创建图
GraphMatrix* createGraph() {
    GraphMatrix* graph = (GraphMatrix*)malloc(sizeof(GraphMatrix));
    if (!graph) {
        return NULL;
    }
    
    graph->size = 0;
    graph->city = NULL;
    graph->graphRoad = NULL;
    graph->graphRailway = NULL;
    graph->graphAir = NULL;
    graph->timeRoad = NULL;
    graph->timeRailway = NULL;
    graph->timeAir = NULL;
    
    return graph;
}

// 读取交通数据文件
GraphMatrix* readTransportData(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    GraphMatrix* graph = createGraph();
    if (!graph) {
        fclose(file);
        return NULL;
    }
    
    char buffer[256];
    char fromCity[50], toCity[50], transportMode[20];
    int distance, cost;
    float time;
    
    // 跳过标题行
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        destroyGraph(graph);
        return NULL;
    }
    
    // 读取每一行数据
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // 跳过空行
        if (strlen(buffer) <= 1) {
            continue;
        }
        
        // 解析数据行
        if (sscanf(buffer, "%s %s %s %d %f %d", fromCity, toCity, transportMode, &distance, &time, &cost) != 6) {
            // 尝试其他可能的格式 - 修复类型匹配问题
            int timeInt;
            if (sscanf(buffer, "%s %s %s %d %d %d", fromCity, toCity, transportMode, &distance, &timeInt, &cost) != 6) {
                printf("Warning: Cannot parse line\n");
                continue;
            }
            time = (float)timeInt; // 转换为float
        }
        
        // 添加城市到图中
        int fromIndex = addCity(graph, fromCity);
        int toIndex = addCity(graph, toCity);
        
        if (fromIndex == -1 || toIndex == -1) {
            printf("Error: Failed to add cities\n");
            fclose(file);
            destroyGraph(graph);
            return NULL;
        }
        
        // 检查矩阵是否已分配
        if (!graph->graphRoad || !graph->graphRailway || !graph->graphAir || 
            !graph->timeRoad || !graph->timeRailway || !graph->timeAir) {
            printf("Error: Matrix not allocated\n");
            fclose(file);
            destroyGraph(graph);
            return NULL;
        }
        
        // 检查索引是否有效
        if (fromIndex >= graph->size || toIndex >= graph->size) {
            printf("Error: Invalid city index\n");
            fclose(file);
            destroyGraph(graph);
            return NULL;
        }
        
        // 根据交通方式设置对应的邻接矩阵值（无向图，添加双向边）
        if (strcmp(transportMode, "自驾") == 0 || strcmp(transportMode, "ROAD") == 0) {
            graph->graphRoad[fromIndex][toIndex] = cost;
            graph->graphRoad[toIndex][fromIndex] = cost; // 添加反向边
            graph->timeRoad[fromIndex][toIndex] = (int)time;
            graph->timeRoad[toIndex][fromIndex] = (int)time; // 添加反向边
        } else if (strcmp(transportMode, "高铁") == 0 || strcmp(transportMode, "RAILWAY") == 0) {
            graph->graphRailway[fromIndex][toIndex] = cost;
            graph->graphRailway[toIndex][fromIndex] = cost; // 添加反向边
            graph->timeRailway[fromIndex][toIndex] = (int)time;
            graph->timeRailway[toIndex][fromIndex] = (int)time; // 添加反向边
        } else if (strcmp(transportMode, "航空") == 0 || strcmp(transportMode, "AIR") == 0) {
            graph->graphAir[fromIndex][toIndex] = cost;
            graph->graphAir[toIndex][fromIndex] = cost; // 添加反向边
            graph->timeAir[fromIndex][toIndex] = (int)time;
            graph->timeAir[toIndex][fromIndex] = (int)time; // 添加反向边
        } else {
            printf("Warning: Unknown transport mode: %s\n", transportMode);
        }
    }
    
    fclose(file);
    return graph;
}

// Dijkstra算法求解最短路径（支持过滤交通方式和多维度）
void dijkstra(GraphMatrix* graph, int start, int* dist, PreInfo* pre, 
             int allowRoad, int allowRailway, int allowAir, enum SearchDimension dimension) {
    int n = graph->size;
    int* visited = (int*)malloc(n * sizeof(int));
    
    // 初始化
    for (int i = 0; i < n; i++) {
        dist[i] = -1; // -1表示无穷大
        pre[i].preVex = -1;
        pre[i].transportMode = ROAD;
        visited[i] = 0;
    }
    
    // 起点到自身的距离为0
    dist[start] = 0;
    
    for (int i = 0; i < n; i++) {
        // 找到当前未访问节点中距离最小的节点
        int minDist = -1;
        int u = -1;
        
        for (int j = 0; j < n; j++) {
            if (!visited[j] && dist[j] != -1) {
                if (minDist == -1 || dist[j] < minDist) {
                    minDist = dist[j];
                    u = j;
                }
            }
        }
        
        if (u == -1) {
            break; // 没有可达节点了
        }
        
        visited[u] = 1;
        
        // 更新从u出发到其他节点的距离
        for (int v = 0; v < n; v++) {
            if (!visited[v]) {
                // 检查三种交通方式
                int minValue = -1;
                enum TransportMode bestMode = ROAD;
                
                // 根据搜索维度和允许的交通方式检查边
                if (allowRoad && graph->graphRoad[u][v] != -1) {
                    int value = (dimension == BY_COST) ? 
                               (dist[u] + graph->graphRoad[u][v]) : 
                               (dist[u] + graph->timeRoad[u][v]);
                    minValue = value;
                    bestMode = ROAD;
                }
                
                if (allowRailway && graph->graphRailway[u][v] != -1) {
                    int value = (dimension == BY_COST) ? 
                               (dist[u] + graph->graphRailway[u][v]) : 
                               (dist[u] + graph->timeRailway[u][v]);
                    if (minValue == -1 || value < minValue) {
                        minValue = value;
                        bestMode = RAILWAY;
                    }
                }
                
                if (allowAir && graph->graphAir[u][v] != -1) {
                    int value = (dimension == BY_COST) ? 
                               (dist[u] + graph->graphAir[u][v]) : 
                               (dist[u] + graph->timeAir[u][v]);
                    if (minValue == -1 || value < minValue) {
                        minValue = value;
                        bestMode = AIR;
                    }
                }
                
                // 如果找到更短的路径，更新
                if (minValue != -1 && (dist[v] == -1 || minValue < dist[v])) {
                    dist[v] = minValue;
                    pre[v].preVex = u;
                    pre[v].transportMode = bestMode;
                }
            }
        }
    }
    
    free(visited);
}

// 获取交通方式的字符串表示
const char* getTransportModeString(enum TransportMode mode) {
    switch (mode) {
        case ROAD:
            return "Road";
        case RAILWAY:
            return "Railway";
        case AIR:
            return "Air";
        default:
            return "Unknown";
    }
}

// 输出最短路径
void printPath(GraphMatrix* graph, int start, int end, int* dist, PreInfo* pre, enum SearchDimension dimension) {
    if (dist[end] == -1) {
        printf("No path found from %s to %s\n", graph->city[start], graph->city[end]);
        return;
    }
    
    printf("\nOptimal route from %s to %s:\n", graph->city[start], graph->city[end]);
    if (dimension == BY_COST) {
        printf("Total cost: %d yuan\n\n", dist[end]);
    } else {
        printf("Total time: %d hours\n\n", dist[end]);
    }
    
    // 使用栈来反转路径顺序
    std::stack<int> cityPath;
    std::stack<enum TransportMode> modePath;
    int current = end;
    
    // 从终点回溯到起点
    while (current != start) {
        cityPath.push(current);
        modePath.push(pre[current].transportMode);
        current = pre[current].preVex;
    }
    
    // 输出路径
    printf("%s", graph->city[start]);
    
    while (!cityPath.empty()) {
        current = cityPath.top();
        enum TransportMode mode = modePath.top();
        cityPath.pop();
        modePath.pop();
        
        printf(" -> %s (%s)", graph->city[current], getTransportModeString(mode));
    }
    printf("\n");
}

// 销毁图
void destroyGraph(GraphMatrix* graph) {
    if (!graph) {
        return;
    }
    
    // 释放城市名称数组
    for (int i = 0; i < graph->size; i++) {
        free(graph->city[i]);
    }
    free(graph->city);
    
    // 释放邻接矩阵
    for (int i = 0; i < graph->size; i++) {
        if (graph->graphRoad) free(graph->graphRoad[i]);
        if (graph->graphRailway) free(graph->graphRailway[i]);
        if (graph->graphAir) free(graph->graphAir[i]);
        if (graph->timeRoad) free(graph->timeRoad[i]);
        if (graph->timeRailway) free(graph->timeRailway[i]);
        if (graph->timeAir) free(graph->timeAir[i]);
    }
    free(graph->graphRoad);
    free(graph->graphRailway);
    free(graph->graphAir);
    free(graph->timeRoad);
    free(graph->timeRailway);
    free(graph->timeAir);
    
    free(graph);
}

// 显示所有城市
void showCities(GraphMatrix* graph) {
    printf("\nAvailable cities:\n");
    printf("----------------\n");
    for (int i = 0; i < graph->size; i++) {
        printf("%d. %s\n", i + 1, graph->city[i]);
    }
    printf("----------------\n");
}

// 实现城市连接的增加功能
void addConnection(GraphMatrix* graph, int startCity, int endCity, enum TransportMode mode, int cost, int time) {
    if (startCity < 0 || startCity >= graph->size || endCity < 0 || endCity >= graph->size) {
        printf("Invalid city index\n");
        return;
    }
    
    switch (mode) {
        case ROAD:
            graph->graphRoad[startCity][endCity] = cost;
            graph->graphRoad[endCity][startCity] = cost; // 无向图，双向添加
            graph->timeRoad[startCity][endCity] = time;
            graph->timeRoad[endCity][startCity] = time;
            break;
        case RAILWAY:
            graph->graphRailway[startCity][endCity] = cost;
            graph->graphRailway[endCity][startCity] = cost; // 无向图，双向添加
            graph->timeRailway[startCity][endCity] = time;
            graph->timeRailway[endCity][startCity] = time;
            break;
        case AIR:
            graph->graphAir[startCity][endCity] = cost;
            graph->graphAir[endCity][startCity] = cost; // 无向图，双向添加
            graph->timeAir[startCity][endCity] = time;
            graph->timeAir[endCity][startCity] = time;
            break;
    }
    
    printf("Connection added: %s to %s (%s), cost: %d yuan, time: %d hours\n", 
           graph->city[startCity], graph->city[endCity], getTransportModeString(mode), cost, time);
}

// 实现城市连接的删除功能
void removeConnection(GraphMatrix* graph, int startCity, int endCity, enum TransportMode mode) {
    if (startCity < 0 || startCity >= graph->size || endCity < 0 || endCity >= graph->size) {
        printf("Invalid city index\n");
        return;
    }
    
    switch (mode) {
        case ROAD:
            graph->graphRoad[startCity][endCity] = -1;
            graph->graphRoad[endCity][startCity] = -1;
            graph->timeRoad[startCity][endCity] = -1;
            graph->timeRoad[endCity][startCity] = -1;
            break;
        case RAILWAY:
            graph->graphRailway[startCity][endCity] = -1;
            graph->graphRailway[endCity][startCity] = -1;
            graph->timeRailway[startCity][endCity] = -1;
            graph->timeRailway[endCity][startCity] = -1;
            break;
        case AIR:
            graph->graphAir[startCity][endCity] = -1;
            graph->graphAir[endCity][startCity] = -1;
            graph->timeAir[startCity][endCity] = -1;
            graph->timeAir[endCity][startCity] = -1;
            break;
    }
    
    printf("Connection removed: %s to %s (%s)\n", 
           graph->city[startCity], graph->city[endCity], getTransportModeString(mode));
}

// 主函数
int main() {
    const char* filename = ".\\transport_data.txt";  // 使用相对路径
    
    printf("Loading transport data from file...\n");
    GraphMatrix* graph = readTransportData(filename);
    
    if (!graph) {
        printf("Failed to load transport data. Exiting...\n");
        return 1;
    }
    
    printf("Successfully loaded data. %d cities in the graph.\n", graph->size);
    
    while (1) {
        printf("\n===== City Transport System =====\n");
        showCities(graph);
        
        printf("\nPlease choose an option:\n");
        printf("1. Find the shortest path\n");
        printf("2. Add a city connection\n");
        printf("3. Remove a city connection\n");
        printf("4. Exit\n");
        
        int choice;
        printf("Your choice: ");
        scanf("%d", &choice);
        
        if (choice == 4) {
            break;
        } else if (choice == 1) {
            char startCity[50], endCity[50];
            printf("\nEnter departure city: ");
            scanf("%s", startCity);
            printf("Enter destination city: ");
            scanf("%s", endCity);
            
            // 查找城市索引
            int startIndex = findCityIndex(graph, startCity);
            int endIndex = findCityIndex(graph, endCity);
            
            if (startIndex == -1) {
                printf("Error: Departure city '%s' not found.\n", startCity);
                continue;
            }
            
            if (endIndex == -1) {
                printf("Error: Destination city '%s' not found.\n", endCity);
                continue;
            }
            
            if (startIndex == endIndex) {
                printf("Departure and destination cities are the same.\n");
                continue;
            }
            
            // 选择搜索维度
            int dimensionChoice;
            printf("\nPlease choose search dimension:\n");
            printf("1. By cost (yuan)\n");
            printf("2. By time (hours)\n");
            printf("Your choice: ");
            scanf("%d", &dimensionChoice);
            
            enum SearchDimension dimension = (dimensionChoice == 2) ? BY_TIME : BY_COST;
            
            // 选择交通方式
            printf("\nPlease select transportation modes (enter the corresponding numbers, separated by spaces):\n");
            printf("1. Road\n");
            printf("2. Railway\n");
            printf("3. Air\n");
            printf("For example, enter '1 2' to use road and railway only\n");
            
            int allowRoad = 0, allowRailway = 0, allowAir = 0;
            int modeInput;
            char c;
            
            // 读取用户选择的交通方式
            while ((c = getchar()) != '\n' && c != EOF); // 清空输入缓冲区
            printf("Your choices: ");
            
            while (scanf("%d", &modeInput) == 1) {
                switch (modeInput) {
                    case 1:
                        allowRoad = 1;
                        break;
                    case 2:
                        allowRailway = 1;
                        break;
                    case 3:
                        allowAir = 1;
                        break;
                    default:
                        printf("Invalid mode choice: %d\n", modeInput);
                }
                
                // 检查是否还有更多输入
                if ((c = getchar()) == '\n') break;
                // 跳过空格
                if (c != ' ') {
                    ungetc(c, stdin);
                }
            }
            
            // 如果用户没有选择任何交通方式，默认使用所有方式
            if (!allowRoad && !allowRailway && !allowAir) {
                printf("No transportation modes selected. Using all modes.\n");
                allowRoad = allowRailway = allowAir = 1;
            }
            
            // 显示用户选择的交通方式
            printf("\nSelected transportation modes:");
            if (allowRoad) printf(" Road");
            if (allowRailway) printf(" Railway");
            if (allowAir) printf(" Air");
            printf("\n");
            
            // 分配内存
            int* dist = (int*)malloc(graph->size * sizeof(int));
            PreInfo* pre = (PreInfo*)malloc(graph->size * sizeof(PreInfo));
            
            if (!dist || !pre) {
                printf("Error: Memory allocation failed.\n");
                free(dist);
                free(pre);
                continue;
            }
            
            // 计算最短路径
            dijkstra(graph, startIndex, dist, pre, allowRoad, allowRailway, allowAir, dimension);
            
            // 输出路径
            printPath(graph, startIndex, endIndex, dist, pre, dimension);
            
            // 释放内存
            free(dist);
            free(pre);
        } else if (choice == 2) {
            // 添加城市连接
            char startCity[50], endCity[50];
            int modeChoice, cost, time;
            printf("\nEnter the starting city: ");
            scanf("%s", startCity);
            printf("Enter the destination city: ");
            scanf("%s", endCity);
            
            // 查找城市索引
            int startIndex = findCityIndex(graph, startCity);
            int endIndex = findCityIndex(graph, endCity);
            
            if (startIndex == -1) {
                printf("Error: Starting city '%s' not found.\n", startCity);
                continue;
            }
            
            if (endIndex == -1) {
                printf("Error: Destination city '%s' not found.\n", endCity);
                continue;
            }
            
            printf("Please select transportation mode:\n");
            printf("1. Road\n");
            printf("2. Railway\n");
            printf("3. Air\n");
            printf("Your choice: ");
            scanf("%d", &modeChoice);
            
            enum TransportMode mode;
            switch (modeChoice) {
                case 1:
                    mode = ROAD;
                    break;
                case 2:
                    mode = RAILWAY;
                    break;
                case 3:
                    mode = AIR;
                    break;
                default:
                    printf("Invalid mode choice\n");
                    continue;
            }
            
            printf("Please enter the cost (yuan): ");
            scanf("%d", &cost);
            printf("Please enter the time (hours): ");
            scanf("%d", &time);
            
            addConnection(graph, startIndex, endIndex, mode, cost, time);
        } else if (choice == 3) {
            // 删除城市连接
            char startCity[50], endCity[50];
            int modeChoice;
            printf("\nEnter the starting city: ");
            scanf("%s", startCity);
            printf("Enter the destination city: ");
            scanf("%s", endCity);
            
            // 查找城市索引
            int startIndex = findCityIndex(graph, startCity);
            int endIndex = findCityIndex(graph, endCity);
            
            if (startIndex == -1) {
                printf("Error: Starting city '%s' not found.\n", startCity);
                continue;
            }
            
            if (endIndex == -1) {
                printf("Error: Destination city '%s' not found.\n", endCity);
                continue;
            }
            
            printf("Please select transportation mode to remove:\n");
            printf("1. Road\n");
            printf("2. Railway\n");
            printf("3. Air\n");
            printf("Your choice: ");
            scanf("%d", &modeChoice);
            
            enum TransportMode mode;
            switch (modeChoice) {
                case 1:
                    mode = ROAD;
                    break;
                case 2:
                    mode = RAILWAY;
                    break;
                case 3:
                    mode = AIR;
                    break;
                default:
                    printf("Invalid mode choice\n");
                    continue;
            }
            
            removeConnection(graph, startIndex, endIndex, mode);
        } else {
            printf("Invalid choice\n");
        }
    }
    
    // 清理资源
    destroyGraph(graph);
    printf("\nProgram exited.\n");
    
    return 0;
}