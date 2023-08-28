/*	
*	This is the main cpp file of a program to find a path through a maze.
*	This program reads a text file representing a maze. After initializing the maze and the graph it represents, the maze will be solved with a function
*	which finds the path using Breadth First Search recursively, and another which finds it non-recursively using queues.
*   The program can also print the maze and the steps taken in the path.
*	The functions added to the base maze class provided are findPathRecursive() and findPathNonRecursive(), in addition to printFunction() and matrix order.
*   Funcions findShortestPath1() and findShortestPath2() are added to find and print the shortest path using BFS and Dijkstra Algorithm, respectively. These
*   functions return true if there is a shortest path and false if no path can be found. If no path can be found they will also print this as the result.
*/

#include <iostream>
#include <limits.h>
#include "d_except.h"
#include <list>
#include <fstream>
#include "d_matrix.h"
#include "graph.h"
#include <queue>

using namespace std;

//define constants to check neighbors
const int x[] = { 1, 0, -1, 0 };
const int y[] = { 0, 1, 0, -1 };

//struct pos to compare maze nodes by weight for priority queue
struct pos {
    int i, j, w, v;
    bool operator <(const pos& other) const {
        return w < other.w;
    }
    bool operator >(const pos& other) const {
        return w > other.w;
    }
};

typedef struct pos Pos;

class maze
{
public:
    maze(ifstream& fin);
    void print(int, int, int, int);
    bool isLegal(int i, int j);

    void setMap(int i, int j, int n);
    int getMap(int i, int j) const;
    Pos posFromMap(int v);
    void mapMazeToGraph(maze& m, graph& g);
    void initializeGraph(maze& m, graph& g);

    int getRows() { return rows; }
    int getCols() { return cols; }

    void findPathRecursive(maze& m, graph& g, int curi, int curj);
    void findPathNonRecursive(maze& m, graph& g, int starti, int startj);

    bool findShortestPath1(maze& m, graph& g, int curi, int curj);
    bool findShortestPath2(maze& m, graph& g, int curi, int curj);

    const bool getPathFound() { return pathFound; }
    void printPath();                                 // prints the path taken by the + using the order matrix
    void printPathRev();

private:
    int rows; // number of rows in the maze
    int cols; // number of columns in the maze

    bool pathFound;

    matrix<bool> value;
    matrix<int> map;      // Mapping from maze (i,j) values to node index values
    matrix<pos> position;
    matrix<int> pred;
    matrix<int> order;    // Matrix for keeping the order of the points to be used in printPath()
};

void maze::setMap(int i, int j, int n)
// Set mapping from maze cell (i,j) to graph node n. 
{
    map[i][j] = n;
    position[i][j].v = n;
    position[i][j].i = i;
    position[i][j].j = j;
}

int maze::getMap(int i, int j) const
// Return mapping of maze cell (i,j) in the graph.
{
    return map[i][j];
}

Pos maze::posFromMap(int v) {
    //  Returns position for value v
    for (int k = 0; k < map.rows(); ++k) {
        for (int z = 0; z < map.cols(); ++z) {
            if (map[k][z] == v) {
                Pos s;
                s.i = k;
                s.j = z;
                return s;
            }
        }
    }
}

maze::maze(ifstream& fin)
// Initializes a maze by reading values from fin.  Assumes that the
// number of rows and columns indicated in the file are correct.
{
    fin >> rows;
    fin >> cols;

    char x;

    value.resize(rows, cols);
    for (int i = 0; i <= rows - 1; i++)
        for (int j = 0; j <= cols - 1; j++)
        {
            fin >> x;
            if (x == 'O')
                value[i][j] = true;
            else
                value[i][j] = false;
        }

    map.resize(rows, cols);
    position.resize(rows, cols);
}

void maze::print(int goalI, int goalJ, int currI, int currJ)
// Print out a maze, with the goal and current cells marked on the
// board.
{
    cout << endl;

    if (goalI < 0 || goalI > rows - 1 || goalJ < 0 || goalJ > cols - 1)
        throw rangeError("Bad value in maze::print");

    if (currI < 0 || currI > rows - 1 || currJ < 0 || currJ > cols - 1)
        throw rangeError("Bad value in maze::print");

    for (int i = 0; i <= rows - 1; i++)
    {
        for (int j = 0; j <= cols - 1; j++)
        {
            if (i == goalI && j == goalJ)
                cout << "*";
            else
                if (i == currI && j == currJ)
                    cout << "+";
                else
                    if (value[i][j])
                        cout << " ";
                    else
                        cout << "X";
        }
        cout << endl;
    }
    cout << endl;
}

bool maze::isLegal(int i, int j)
// Return the value stored at the (i,j) entry in the maze.
{
    if (i == -1 || i == rows || j == -1 || j == cols) {
        return false;
    }
    else if (i < 0 || i > rows - 1 || j < 0 || j > cols - 1) {
        throw rangeError("Bad value in maze::isLegal");
    }

    return value[i][j];
}

void maze::mapMazeToGraph(maze& m, graph& g)
// Create a graph g that represents the legal moves in the maze m.
{
    //create nodes for legal positions
    int curNode, nextNode;
    for (int i = 0; i <= rows - 1; i++) {
        for (int j = 0; j <= cols - 1; j++)
        {
            if (isLegal(i, j) == true) {
                setMap(i, j, g.addNode(1));
            }
        }
    }

    //create edges for legal moves accross rows
    for (int i = 0; i < rows - 1; i++) {
        for (int j = 0; j <= cols - 1; j++)
        {
            if (isLegal(i, j) == true) {
                curNode = map[i][j];
                if (isLegal(i + 1, j) == true) {
                    nextNode = map[i + 1][j];
                    g.addEdge(curNode, nextNode, 1);
                    g.addEdge(nextNode, curNode, 1);
                }
            }
        }
    }

    //create edges for legal moves accross columns 
    for (int i = 0; i <= rows - 1; i++) {
        for (int j = 0; j < cols - 1; j++)
        {
            if (isLegal(i, j) == true) {
                curNode = map[i][j];
                if (isLegal(i, j + 1) == true) {
                    nextNode = map[i][j + 1];
                    g.addEdge(curNode, nextNode, 1);
                    g.addEdge(nextNode, curNode, 1);
                }
            }
        }
    }
}

void maze::initializeGraph(maze& m, graph& g) {
    //set up the graph and maze variables so they are ready to solve a new puzzle 
    g.clearVisit();             //set all nodes in graph to unvisited
    order.resize(2, 0);         //set thes size storing the positions of each move
    pathFound = false;          //set solved bool to false
    order[0].clear();
    order[1].clear();
}

// Functions which prints the path taken by the + using the order matrix
void maze::printPath() {
    for (int i = 0; i < order[1].size(); i++) {
        print(rows - 1, cols - 1, order[0][i], order[1][i]);
    }
}

void maze::printPathRev() {
    // prints the path by printing the order in reverse 
    for (int i = order[1].size() - 1; i >= 0; i--) {
        print(rows - 1, cols - 1, order[0][i], order[1][i]);
    }
}

//old functions from 5a
void maze::findPathRecursive(maze& m, graph& g, int curi, int curj) {
    //recursive dfs search to find path in maze
    int v = map[curi][curj];
    order[0].push_back(curi);
    order[1].push_back(curj);
    g.visit(v);

    //base case
    if (v == map[rows - 1][cols - 1]) {
        pathFound = true;
        return;
    }

    //check all neighbors for possible move
    for (int i = 0; i < 4; i++) {
        if (isLegal(curi + x[i], curj + y[i]) == true) {
            if (g.isVisited(v, map[curi + x[i]][curj + y[i]]) == false) {
                g.visit(v, map[curi + x[i]][curj + y[i]]);
                m.findPathRecursive(m, g, curi + x[i], curj + y[i]);
                if (pathFound == true) {
                    return;
                }
            }
        }
    }
    return;
}

void maze::findPathNonRecursive(maze& m, graph& g, int starti, int startj) {
    // Do we need a function for reseting all nodes in the graph to unvisited?

    // initialize queue
    queue<int> qi;
    queue<int> qj;

    g.clearVisit();             //set all nodes in graph to unvisited
    order.resize(2, 0);         //set thes size storing the positions of each move
    pathFound = false;          //set solved bool to false
    order[0].clear();
    order[1].clear();

    // map starting vertex and visit, initialize variables for current index
    int v = map[starti][startj];
    g.visit(v);
    int currenti = starti;
    int currentj = startj;

    // place starting vertex in queue
    qi.push(currenti);
    qj.push(currentj);
    order[0].push_back(currenti);
    order[1].push_back(currentj);

    // queue loop while queue is not empty
    while (!qi.empty() || !qj.empty())
    {
        // set v = to item at front of q
        currenti = qi.front();
        currentj = qj.front();
        v = map[currenti][currentj];

        // check if final destination is reached first
        if (v == map[rows - 1][cols - 1]) {
            pathFound = true;
            break;
        }
        // for each legal unvisited neighbor of vertex v, visit and push into q
        for (int i = 0; i < 4; i++) {
            if (isLegal(currenti + x[i], currentj + y[i]) == true) {
                if (g.isVisited(v, map[currenti + x[i]][currentj + y[i]]) == false) { // are we using isVisited(i,j) right? I am not sure since it seems to be taking an index i,j in the graph header but we are giving it 2 different maps
                    g.visit(v, map[currenti + x[i]][currentj + y[i]]);  // visit neighbor
                    qi.push(currenti + x[i]);  // push into queue
                    qj.push(currentj + y[i]);
                    order[0].push_back(currenti + x[i]);   // giving bad row index error when adding x[i]
                    order[1].push_back(currentj + y[i]);
                    break;
                }
                if (map[currenti + x[i]][currentj + y[i]] == map[rows - 1][cols - 1]) {
                    pathFound = true;
                    break;
                }
            }
        }
        // pop current front and restart the while loop
        qi.pop();
        qj.pop();
    }

}

bool maze::findShortestPath1(maze& m, graph& g, int curi, int curj) {
    //Find shortest path using BFS

    // initialize queue
    queue<int> qi;
    queue<int> qj;
    pred.resize(2, 0);
    pred[0].resize(g.numNodes());
    pred[1].resize(g.numNodes());

    // map starting vertex and visit, initialize variables for current index
    int v = map[curi][curj];
    g.visit(v);
    int currenti = curi;
    int currentj = curj;
    int goal = map[rows - 1][cols - 1];
    pred[0][v] = -1;
    pred[1][v] = -1;

    // place starting vertex in queue
    qi.push(currenti);
    qj.push(currentj);

    // queue loop while queue is not empty
    while (!qi.empty() || !qj.empty())
    {
        // set v = to item at front of q
        currenti = qi.front();
        currentj = qj.front();
        v = map[currenti][currentj];

        if (pathFound) {
            break;
        }

        // for each legal unvisited neighbor of vertex v, visit and push into q
        for (int i = 0; i < 4; i++) {
            if (isLegal(currenti + x[i], currentj + y[i]) == true) {
                if (map[currenti + x[i]][currentj + y[i]] == goal) {
                    v = map[currenti + x[i]][currentj + y[i]];
                    pred[0][v] = currenti;
                    pred[1][v] = currentj;
                    pathFound = true;
                    break;
                }
                if (g.isVisited(map[currenti + x[i]][currentj + y[i]]) == false) {
                    g.visit(map[currenti + x[i]][currentj + y[i]]);  // visit neighbor 
                    qi.push(currenti + x[i]);  // push into queue
                    qj.push(currentj + y[i]);
                    v = map[currenti + x[i]][currentj + y[i]];
                    pred[0][v] = currenti;
                    pred[1][v] = currentj;
                }

            }
        }
        // pop current front and restart the while loop
        qi.pop();
        qj.pop();
    }

    if (pathFound) {
        //print results
        int nextNode = goal;
        while (pred[0][nextNode] != -1) {
            order[0].push_back(pred[0][nextNode]);
            order[1].push_back(pred[1][nextNode]);
            nextNode = map[pred[0][nextNode]][pred[1][nextNode]];
        }
        m.printPathRev();
        return true;
    }
    else {
        cout << "No path found.\n";
        return false;
    }
}

bool maze::findShortestPath2(maze& m, graph& g, int curi, int curj) {
    //initialize variables
    priority_queue<pos> pq;
    matrix<int> pred;
    int goal = map[rows - 1][cols - 1];
    int w, currenti, currentj;
    pos& vec = position[curi][curj];

    //initialize matrix for parent information
    pred.resize(2, 0);
    pred[0].resize(g.numNodes());
    pred[1].resize(g.numNodes());
    pred[0][vec.v] = -1;
    pred[1][vec.v] = -1;

    //push all nodes to priority queue and set to infinate
    for (int i = 0; i <= rows - 1; i++) {
        for (int j = 0; j <= cols - 1; j++)
        {
            if (isLegal(i, j)) {
                g.setNodeWeight(position[i][j].v, -1);
                position[i][j].w = -1;
                pq.push(position[i][j]);
            }
        }
    }

    //set initial node weight
    g.setNodeWeight(vec.v, 0);
    vec.w = 0;

    while (!pq.empty()) {
        //main loop for Dijkstras Algorithm
            //initialize necessary varibles 
        priority_queue<pos> tempq;
        vec = pq.top();
        pq.pop();
        g.visit(vec.v);
        currenti = vec.i;
        currentj = vec.j;

        //check if path found
        if (vec.v == goal) {
            pathFound = true;
        }
        else {
            // check all neighbors update shortest paths
            for (int i = 0; i < 4; i++) {
                if (isLegal(currenti + x[i], currentj + y[i]) == true) {
                    pos& w = position[currenti + x[i]][currentj + y[i]];
                    if (g.isVisited(w.v) == false) {
                        if (g.getNodeWeight(w.v) == -1) {
                            g.setNodeWeight(w.v, g.getNodeWeight(vec.v) + 1); //adds 1 for unweighted maze
                            w.w = (vec.w + 1);
                            pred[0][w.v] = currenti;
                            pred[1][w.v] = currentj;
                        }
                        else if (g.getNodeWeight(w.v) > g.getNodeWeight(vec.v) + g.getNodeWeight(w.v)) {
                            g.setNodeWeight(w.v, g.getNodeWeight(vec.v) + g.getNodeWeight(w.v));
                            w.w = (vec.w + w.w);
                            pred[0][w.v] = currenti;
                            pred[1][w.v] = currentj;
                        }
                    }
                }
            }
        }

        //update the queue with new vals
        pos t1, t2;
        while (!pq.empty()) {
            t1 = pq.top();
            pq.pop();
            t2 = position[t1.i][t1.j];
            tempq.push(t2);
        }
        while (!tempq.empty()) {
            pq.push(tempq.top());
            tempq.pop();
        }
    }

    if (pathFound) {
        //print results
        int nextNode = goal;
        while (pred[0][nextNode] != -1) {
            order[0].push_back(pred[0][nextNode]);
            order[1].push_back(pred[1][nextNode]);
            nextNode = map[pred[0][nextNode]][pred[1][nextNode]];
        }
        m.printPathRev();
        return true;
    }
    else {
        cout << "No path found.\n";
        return false;
    }
}

int main()
{
    char x;
    ifstream fin;

    // Read the maze from the file.
    string fileName = "maze1.txt";

    fin.open(fileName.c_str());
    if (!fin)
    {
        cerr << "Cannot open " << fileName << endl;
        exit(1);
    }

    try
    {
        graph g;
        bool pathIs;
        while (fin && fin.peek() != 'Z')
        {
            maze m(fin);
            m.mapMazeToGraph(m, g);

            //functions to find bfs shortest path
            cout << "===================================================" << endl << endl;

            cout << "Finding shortest path with BFS:\n";
            m.initializeGraph(m, g);           //initialze
            pathIs = m.findShortestPath1(m, g, 0, 0);   //function call
            cout << endl << "Path found (true/false): " << pathIs << endl << endl;

            cout << "===================================================" << endl << endl;

            //functions to find Dijkstra Algorithm shortest path
            cout << "Finding shortest path with Dijkstra Algortithm:\n";
            m.initializeGraph(m, g);           //initialize 
            pathIs = m.findShortestPath2(m, g, 0, 0);   //function call
            cout << endl << "Path found (true/false): " << pathIs << endl << endl;
        }

    }
    catch (indexRangeError& ex)
    {
        cout << ex.what() << endl; exit(1);
    }
    catch (rangeError& ex)
    {
        cout << ex.what() << endl; exit(1);
    }
}