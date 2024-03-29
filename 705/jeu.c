#include <GL/freeglut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdbool.h> // 引入布尔类型支持
#include <time.h> // 包含 time 函数的声明
#include <stdlib.h> // 包含 srand 和 rand 函数的声明
#include <stdio.h> // 包含 sprintf 函数的声明

/***********************************************************************基本参数*****************************************************************************/


// 定义游戏区域和外围区域的尺寸
const int gameAreaWidth = 600;  // 游戏区域宽度
const int gameAreaHeight = 800; // 游戏区域高度
const int borderSize = 100;     // 边界大小

// 总窗口尺寸
const int windowWidth = gameAreaWidth + 2 * borderSize;
const int windowHeight = gameAreaHeight + 2 * borderSize;

// 定义游戏板大小
#define ROWS 8
#define COLS 6

// 游戏板数据结构
int gameBoard[ROWS][COLS] = {{0}};

// 生成新方块的间隔时间（毫秒）
#define GENERATE_INTERVAL 10000
// 倒计时变量，单位毫秒
float countdown = 100000;

// 方块大小
#define BLOCK_SIZE 92

// 定义格子间的空隙大小
#define GAP_SIZE 7.5

// 初始化函数
void init() {
    glClearColor(0.0, 0.0, 0.0, 1.0); // 设置背景颜色
}

// 全局变量存储游戏结束时的最大分数
int finalMaxScore = 0;

// 游戏是否已开始
bool gameStarted = false;
bool showTimerBar = false;
bool gameOver = false;


//颜色设置
float colors[20][3] = {
    {1.0, 0.0, 0.0},   // 红色
    {0.0, 1.0, 0.0},   // 绿色
    {0.0, 0.0, 1.0},   // 蓝色
    {1.0, 1.0, 0.0},   // 黄色
    {1.0, 0.0, 1.0},   // 紫色
    {0.0, 1.0, 1.0},   // 青色
    {1.0, 1.0, 1.0},   // 白色
    {1.0, 0.5, 0.0},   // 橙色
    {1.0, 0.75, 0.8},  // 粉色
    {0.0, 0.0, 0.55},  // 深蓝
    {0.5, 0.5, 0.0},   // 橄榄绿
    {0.6, 0.4, 0.12},  // 棕色
    {0.5, 0.5, 0.5},   // 灰色
    {0.53, 0.81, 0.98},// 亮蓝
    {0.55, 0.0, 0.0},  // 深红
    {0.0, 0.39, 0.0},  // 深绿
    {0.58, 0.0, 0.82}, // 紫罗兰
    {0.68, 0.85, 0.9}, // 淡蓝
    {1.0, 0.84, 0.0},   // 金色
    {0.0, 0.0, 0.0}    // 黑色
};


bool isDragging = false; // 是否正在拖动
int draggedBlockX = -1;  // 被拖动方块的列
int draggedBlockY = -1;  // 被拖动方块的行
int mouseX = 0, mouseY = 0; // 鼠标的屏幕坐标

/***************************************************************更新游戏状态******************************************************************/

/*获取最大分数*/
int getMaxScore() {
    // 如果游戏结束，返回保存的最大分数
    if (gameOver) {
        return finalMaxScore;
    }

    int maxScore = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (gameBoard[i][j] > maxScore) {
                maxScore = gameBoard[i][j];
            }
        }
    }
    return maxScore;
}
/*生成新方块：向上平移*/
void shiftBlocksUp() {
    for (int i = ROWS-1; i >= 0; i--) {
        for (int j = 0; j < COLS; j++) {
            gameBoard[i][j] = gameBoard[i-1][j];
        }
    }
    for (int j = 0; j < COLS; j++) {
           gameBoard[0][j] = 0;
       }
}

/*生成新方块：不生成和上方数字相同的方块*/
int generateRandomBlock(int maxVal, int exclude) {
    int newVal;
    do {
        newVal = rand() % maxVal + 1;
    } while (newVal == exclude);
    return newVal;
}

/*生成新方块：底部生成*/
void generateNewRow() {
    // 检查最上方一排是否有方块
    for (int j = 0; j < COLS; j++) {
        if (gameBoard[ROWS - 1][j] != 0) {
            // 游戏结束，设置 gameOver 为 true 并保存最大分数
            gameOver = true;
            finalMaxScore = getMaxScore(); // 保存游戏结束时的最大分数
            return;
        }
    }

    // 向上移动现有方块
    shiftBlocksUp();

    // 如果游戏已结束，则不生成新方块
    if (gameOver) {
        return;
    }

    // 获取当前最大分数
    int maxScore = getMaxScore();

    // 为最下方一排生成新方块
    for (int j = 0; j < COLS; j++) {
        int exclude = (ROWS > 1) ? gameBoard[1][j] : 0;
        gameBoard[0][j] = generateRandomBlock(maxScore - 2, exclude);
    }
}


/*初始化游戏*/
void initializeGame() {
    gameStarted = true;
    showTimerBar = true; // 显示进度条
    countdown = GENERATE_INTERVAL; // 初始化倒计时

    // 清空游戏板
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            gameBoard[i][j] = 0;
        }
    }
    // 生成两排随机方块，确保上下方块数字不同
    int maxVal = 6; // 假设方块的最大值为6
    for (int j = 0; j < COLS; j++) {
        gameBoard[1][j] = rand() % maxVal + 1; // 为倒数第二行生成随机方块
        gameBoard[0][j] = generateRandomBlock(maxVal, gameBoard[1][j]); // 为最后一行生成随机方块
    }
}


/*鼠标操作*/
void mouse(int button, int state, int x, int y) {
    int transformedY = windowHeight - y;

    if (!gameStarted) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            // 检查是否点击了开始按钮
            if (x >= 300 && x <= 500 && transformedY >= 450 && transformedY <= 550) {
                gameStarted = true;
                countdown = 0;
                initializeGame();
            }
        }
    } else if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            // 开始拖拽
            draggedBlockX = (x - borderSize) / (BLOCK_SIZE + GAP_SIZE);
            draggedBlockY = (transformedY - borderSize) / (BLOCK_SIZE + GAP_SIZE);

            int draggedValue = gameBoard[draggedBlockY][draggedBlockX];

            // 检查是否可以拖动（如果周围有空格或者相同值的方块）
            bool canDrag = false;
            if (draggedBlockY < ROWS - 1 && (gameBoard[draggedBlockY + 1][draggedBlockX] == 0 || gameBoard[draggedBlockY + 1][draggedBlockX] == draggedValue)) {
                canDrag = true;
            }
            if (draggedBlockX > 0 && (gameBoard[draggedBlockY][draggedBlockX - 1] == 0 || gameBoard[draggedBlockY][draggedBlockX - 1] == draggedValue)) {
                canDrag = true;
            }
            if (draggedBlockX < COLS - 1 && (gameBoard[draggedBlockY][draggedBlockX + 1] == 0 || gameBoard[draggedBlockY][draggedBlockX + 1] == draggedValue)) {
                canDrag = true;
            }

            if (canDrag) {
                isDragging = true;
            }
        } else if (state == GLUT_UP && isDragging) {
            isDragging = false;

            // 计算放置的格子位置
            int dropX = (x - borderSize) / (BLOCK_SIZE + GAP_SIZE);
            int dropY = (transformedY - borderSize) / (BLOCK_SIZE + GAP_SIZE);

            int draggedValue = gameBoard[draggedBlockY][draggedBlockX];
            gameBoard[draggedBlockY][draggedBlockX] = 0; // 清空原位置

            // 自动下落直到找到底部或者另一个方块
            while (dropY > 0 && (gameBoard[dropY - 1][dropX] == 0 || gameBoard[dropY - 1][dropX] == draggedValue)) {
                dropY--;
            }

            // 合并或放置方块
            if (gameBoard[dropY][dropX] == draggedValue) {
                gameBoard[dropY][dropX]++; // 合并方块
            } else if (gameBoard[dropY][dropX] == 0) {
                gameBoard[dropY][dropX] = draggedValue; // 放置方块
            } else {
                // 恢复原位置的方块（如果目标位置不为空，且不与拖动的方块值相同）
                gameBoard[draggedBlockY][draggedBlockX] = draggedValue;
            }

            // 重置拖动状态
            draggedBlockX = -1;
            draggedBlockY = -1;
        }
    }
}












void motion(int x, int y) {
    mouseX = x;
    mouseY = windowHeight - y; // 转换 y 坐标
    glutPostRedisplay(); // 请求重绘
    printf("Mouse Position: (%d, %d)\n", mouseX, mouseY);
}


/*键盘操作*/
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // 按 ESC 退出
        exit(0);
    }
}

/*更新游戏状态*/
void updateGame() {
    // 检查顶部是否有方块
    for (int j = 0; j < COLS; j++) {
        if (gameBoard[0][j] != 0) {
            // 游戏结束逻辑
            break;
        }
    }

    glutPostRedisplay();
}






/************************************************************************游戏规则*******************************************************************/


void mergeVerticalBlocks() {
    for (int col = 0; col < COLS; col++) {
        for (int row = ROWS - 1; row > 0; row--) { // 从底部开始向上检查
            if (gameBoard[row][col] != 0 && gameBoard[row][col] == gameBoard[row - 1][col]) {
                // 合并方块
                gameBoard[row][col]++;
                gameBoard[row - 1][col] = 0;

                // 合并后，上方方块可能会导致新的合并，继续检查
                for (int checkRow = row - 1; checkRow > 0; checkRow--) {
                    if (gameBoard[checkRow][col] == gameBoard[checkRow - 1][col]) {
                        gameBoard[checkRow][col]++;
                        gameBoard[checkRow - 1][col] = 0;
                    } else {
                        break; // 不需要进一步检查
                    }
                }
            }
        }
    }
}

void riseBlocksIfEmptyAbove(int gameBoard[ROWS][COLS]) {
    // 从第二行开始向上遍历（最顶行无需检查）
    for (int i = 1; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            // 当前位置有方块且上方为空时
            while (gameBoard[i][j] != 0 && gameBoard[i - 1][j] == 0) {
                // 将方块上移
                gameBoard[i - 1][j] = gameBoard[i][j];
                gameBoard[i][j] = 0;

                // 继续检查上一行，直到遇到非空位置或到达顶部
                if (i > 1) {
                    i--;
                } else {
                    break; // 到达顶部
                }
            }
        }
    }
}







/****************************************************************************计数器代码******************************************************************/




//方块刷新计时器
void onTimer(int value) {
    if (!gameStarted) {
    	 countdown = 0;
    	 glutTimerFunc(10000, onTimer, 0);
        return; // 如果游戏未开始，直接返回
    }
        countdown -= 10000; // 减少倒计时

        if (countdown <= 0) {
            generateNewRow();  // 生成新的一行方块
            updateGame();      // 更新游戏状态
            countdown = GENERATE_INTERVAL; // 重置倒计时
        }

        glutPostRedisplay();

        glutTimerFunc(10000, onTimer, 0);
}


//进度条计时器
void updateTimerBar(int value) {
    if (!gameStarted) {
    	countdown = 0;
    	glutTimerFunc(20, updateTimerBar, 0);
        return; // 如果游戏未开始，直接返回
    }
    countdown -= 20;
    if (countdown <= 0) {
        countdown = GENERATE_INTERVAL; // 重置倒计时
    }

    glutPostRedisplay();

    glutTimerFunc(20, updateTimerBar, 0); // 每20毫秒调用一次更新进度条
}


/***********************************************************************绘制代码**************************************************************************/


/*绘制方块*/

void drawSquare(float x, float y, float size) {
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + size, y);
        glVertex2f(x + size, y + size);
        glVertex2f(x, y + size);
    glEnd();
}

/*绘制文本*/
void drawText(const char *text, float x, float y) {
    glRasterPos2f(x, y);
    for (const char *c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

/*绘制被拖拽的方块*/
void drawBlockAtScreenPosition(int value, int screenX, int screenY) {
    float x = screenX - BLOCK_SIZE / 2.0f;
    float y = screenY - BLOCK_SIZE / 2.0f;

    if (value > 0 && value <= 20) {
        glColor3f(colors[value-1][0], colors[value-1][1], colors[value-1][2]);
    } else {
        glColor3f(0.5, 0.5, 0.5); // 默认颜色
    }

    drawSquare(x, y, BLOCK_SIZE);

    // 设置文本颜色（例如黑色）
    glColor3f(0.0, 0.0, 0.0);
    char numStr[3];
    sprintf(numStr, "%d", value);
    drawText(numStr, x + (BLOCK_SIZE / 2.2), y + (BLOCK_SIZE / 2.2));
}



/*绘制开始按钮*/
void drawStartButton() {
    // 绘制按钮
    glColor3f(0.968627f, 0.862745f, 0.435294f); // 奶油白色按钮
    glBegin(GL_QUADS);
        glVertex2f(300, 450);
        glVertex2f(500, 450);
        glVertex2f(500, 550);
        glVertex2f(300, 550);
    glEnd();

    // 设置文字颜色并绘制开始游戏按钮上的文字
    glColor3f(0.490196f, 0.235294f, 0.596078f); // 富有的紫色文本
    drawText("Commencer", 345, 485); // 假设文本在按钮中间

    // 设置深灰色并在按钮上方绘制游戏说明
    glColor3f(0.3f, 0.3f, 0.3f); // 深灰色文本
    drawText("Welcome to Twenty!", 300, 700); // 标题
    drawText("To play, pick up tiles and drop them onto tiles", 200, 620); // 说明第一部分
    drawText("with the same value", 300, 560); // 说明第二部分
}

/*绘制方块逻辑*/
void drawBlock(int value, int row, int col) {
    // 考虑空隙，重新计算 x 和 y 坐标
    float x = borderSize + col * (BLOCK_SIZE + GAP_SIZE) + GAP_SIZE / 2.0f;
    float y = borderSize + row * (BLOCK_SIZE + GAP_SIZE) + GAP_SIZE / 2.0f;

    // 根据方块的值选择颜色
    if (value > 0 && value <= 20) {
        glColor3f(colors[value-1][0], colors[value-1][1], colors[value-1][2]);
    } else {
        glColor3f(0.5, 0.5, 0.5); // 默认颜色
    }

    drawSquare(x, y, BLOCK_SIZE); // 绘制方块

    // 设置文本颜色（例如黑色）
    glColor3f(0.0, 0.0, 0.0);

    // 将数字转换为字符串
    char numStr[3];
    sprintf(numStr, "%d", value);

    // 调整文本位置，使其居中显示在方块上
    drawText(numStr, x + (BLOCK_SIZE / 2.2), y + (BLOCK_SIZE / 2.2));
}

/*进度条绘制*/
void drawTimerBar() {
    if (!showTimerBar) return; // 如果不显示进度条，则直接返回

    float barWidth = (countdown / GENERATE_INTERVAL) * windowWidth;
    glColor3f(1.0f, 0.0f, 0.0f); // 红色时间条
    glBegin(GL_QUADS);
        glVertex2f(windowWidth - barWidth, windowHeight - 20);
        glVertex2f(windowWidth, windowHeight - 20);
        glVertex2f(windowWidth, windowHeight - 30);
        glVertex2f(windowWidth - barWidth, windowHeight - 30);
    glEnd();
}



/*绘制游戏画面逻辑*/
/*绘制游戏画面逻辑*/
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 设置正投影
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);

    // 绘制外围区域背景
    glColor3f(1.0, 1.0, 1.0); // 白色背景
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(windowWidth, 0);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(0, windowHeight);
    glEnd();

    // 绘制游戏区域背景
    glColor3f(0.8, 0.8, 0.8); // 灰色背景
    glBegin(GL_QUADS);
        glVertex2f(borderSize, borderSize);
        glVertex2f(borderSize + gameAreaWidth, borderSize);
        glVertex2f(borderSize + gameAreaWidth, borderSize + gameAreaHeight);
        glVertex2f(borderSize, borderSize + gameAreaHeight);
    glEnd();

    // 如果游戏结束
    if (gameOver) {
        glColor3f(1.0, 0.0, 0.0); // 设置红色文本
        drawText("Game Over", windowWidth / 2 - 50, windowHeight / 2 + 20); // 居中显示游戏结束信息

        char scoreStr[20];
        sprintf(scoreStr, "Final Score: %d", finalMaxScore);
        drawText(scoreStr, windowWidth / 2 - 60, windowHeight / 2 - 20); // 显示最终得分
    } else {
        // 在游戏进行中的绘制逻辑
        mergeVerticalBlocks();
        riseBlocksIfEmptyAbove(gameBoard);

        if (!gameStarted) {
            drawStartButton();
        } else {
            // 绘制游戏板上的方块和数字
            for (int i = 0; i < ROWS; i++) {
                for (int j = 0; j < COLS; j++) {
                    if (gameBoard[i][j] != 0) {
                        drawBlock(gameBoard[i][j], i, j);
                    }
                }
            }

            // 如果正在拖动方块，则绘制被拖动的方块
            if (isDragging) {
                int blockValue = gameBoard[draggedBlockY][draggedBlockX];
                drawBlockAtScreenPosition(blockValue, mouseX, mouseY);
            }
        }

        if (gameStarted) {
            // 获取最大分数
            int maxScore = getMaxScore();

            // 将分数转换为字符串
            char scoreStr[20];
            sprintf(scoreStr, "Score: %d", maxScore);

            // 设置文本颜色
            glColor3f(1.0, 0.0, 1.0);

            // 绘制分数文本
            int textLength = strlen(scoreStr) * 10;
            drawText(scoreStr, windowWidth - textLength - 20, windowHeight - 50); // 在窗口右上角绘制文本
        }

        // 绘制时间条
        drawTimerBar();
    }

    glFlush();
    glutSwapBuffers();
}








/***************************************************************************main*******************************************************************/







// 主函数
int main(int argc, char** argv) {
	srand(time(NULL)); // 初始化随机数生成器

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Twenty Game");

    init(); // 初始化设置


    // 注册回调函数
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);   // 注册鼠标移动事件处理函数
    glutKeyboardFunc(keyboard);
    glutIdleFunc(updateGame);

    // 设置定时器
    glutTimerFunc(10000, onTimer, 0); // 传递100作为间隔时间参数

    // 设置进度条更新的定时器
    glutTimerFunc(20, updateTimerBar, 0); // 每20毫秒调用一次updateTimerBar

    glutMainLoop();
    return 0;
}
