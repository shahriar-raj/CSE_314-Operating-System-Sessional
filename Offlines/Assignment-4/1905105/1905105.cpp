#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <syncstream>

using namespace std;

#define PS (stnum % 4) + 1
#define FINISHED 3
#define WAITING 1
#define PRINTING 2

int N, M, G, w, x, y;
int no_of_submissions;
int rc = 0;
vector<int> students;
int *state;
bool Occupied[4];
vector<int> groups;
chrono::time_point<std::chrono::high_resolution_clock> start_time;
sem_t S[4];
sem_t *stu_S;
sem_t B;
sem_t *GL;
sem_t bin_sem;
sem_t mu;
sem_t e_b;
ofstream outputFile("output.txt");

void test(int stnum)
{
    if (state[stnum - 1] == WAITING && !Occupied[PS - 1])
    {
        state[stnum - 1] = PRINTING;
        Occupied[PS - 1] = true;
        sem_post(&stu_S[stnum - 1]);
    }
}

void print(int stnum)
{
    sem_wait(&mu);
    state[stnum - 1] = WAITING;
    test(stnum - 1);
    sem_post(&mu);
    sem_wait(&stu_S[stnum - 1]);
}

void finish(int stnum)
{
    sem_wait(&mu);
    state[stnum - 1] = FINISHED;
    Occupied[PS - 1] = false;
    for (int i = 0; i < N; i++)
    {
        if (state[i] == WAITING && ((i / 4) == (stnum - 1) / 4) && PS == ((i + 1) % 4) + 1)
        {
            test(i + 1);
        }
    }
    for (int i = 0; i < N; i++)
    {
        if (state[i] == WAITING && PS == ((i + 1) % 4) + 1)
        {
            test(i + 1);
        }
    }
    sem_post(&mu);
}

void *student(void *num)
{
    int *i = (int *)num;
    int stnum = *i;

    print(*i); // try printing

    sem_wait(&S[PS]);
    sleep(w);
    chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end_time - start_time;
    osyncstream(outputFile) << "Student " << stnum << " has finished printing at time " << (int)duration.count() << endl;
    groups[(stnum - 1) / M] += 1;
    // cout << groups[(stnum - 1) / M] << endl;
    sem_post(&S[PS]);

    finish(*i); // finishing

    if (groups[(stnum - 1) / M] == M)
    {
        sem_post(&GL[(stnum - 1) / M]);
    }
    if ((((stnum - 1) / M) + 1) * M == stnum)
    {
        sem_wait(&GL[(stnum - 1) / M]);
        end_time = std::chrono::high_resolution_clock::now();
        duration = end_time - start_time;
        osyncstream(outputFile) << "Group " << ((stnum - 1) / M) + 1 << " has finished printing at time " << (int)duration.count() << endl;
        sleep(2);
        sem_wait(&B);
        end_time = std::chrono::high_resolution_clock::now();
        duration = end_time - start_time;
        osyncstream(outputFile) << "Group " << ((stnum - 1) / M) + 1 << " has started binding at time " << (int)duration.count() << endl;
        sleep(x);
        end_time = std::chrono::high_resolution_clock::now();
        duration = end_time - start_time;
        osyncstream(outputFile) << "Group " << ((stnum - 1) / M) + 1 << " has finished binding at time " << (int)duration.count() << endl;
        sem_post(&B);
        sem_wait(&e_b);
        sleep(y);
        end_time = std::chrono::high_resolution_clock::now();
        duration = end_time - start_time;
        no_of_submissions++;
        osyncstream(outputFile) << "Group " << ((stnum - 1) / M) + 1 << " has submitted the report at time " << (int)duration.count() << endl;
        // sleep(y);
        sem_post(&e_b);
        sem_post(&GL[(stnum - 1) / M]);
    }
    pthread_exit(NULL);
}

void *reader(void *num)
{
    double lambda = 3.0;
    default_random_engine generator;
    poisson_distribution<int> distribution(lambda);
    while (true)
    {
        sleep(distribution(generator));
        int *i = (int *)num;
        sem_wait(&bin_sem);
        rc = rc + 1;
        if (rc == 1)
        {
            sem_wait(&e_b);
        }
        sem_post(&bin_sem);
        chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end_time - start_time;
        osyncstream(outputFile) << "Staff " << *i << " has started reading the entry book at time " << (int)duration.count() << " No. of Submissions: " << no_of_submissions << endl;
        sleep(y);
        sem_wait(&bin_sem);
        rc = rc - 1;
        if (rc == 0)
        {
            sem_post(&e_b);
        }
        sem_post(&bin_sem);
        if (no_of_submissions == G)
        {
            break;
        }
    }
    pthread_exit(NULL);
}

int main()
{
    double lambda = 3.0;
    default_random_engine generator;
    poisson_distribution<int> distribution(lambda);
    int stuffs[2] = {1, 2};
    ifstream inputFile("input.txt"); // Input from file

    if (!inputFile.is_open())
    {
        cout << "Could not open the file." << endl;
        return 1;
    }

    inputFile >> N >> M;
    inputFile >> w >> x >> y;

    inputFile.close();
    // cout << N << M << w << x << y << endl;
    G = N / M;
    GL = new sem_t[G];
    state = new int[N];
    stu_S = new sem_t[N];
    for (int i = 0; i < N; i++)
    {
        sem_init(&stu_S[i], 0, 1);
    }
    for (int i = 0; i < G; i++)
    {
        sem_init(&GL[i], 0, 1);
        sem_wait(&GL[i]);
    }

    sem_init(&bin_sem, 0, 1);
    sem_init(&e_b, 0, 1);
    sem_init(&mu, 0, 1);
    for (int i = 0; i < N; i++)
    {
        students.push_back(i + 1);
    }

    for (int i = 0; i < G; i++)
    {
        groups.push_back(0);
    }

    start_time = chrono::high_resolution_clock::now();
    pthread_t thread_id[N];

    for (int i = 0; i < 4; i++)
    {
        sem_init(&S[i], 0, 1);
    }

    sem_init(&B, 0, 2);
    shuffle(students.begin(), students.end(), generator);

    for (int i = 0; i < 2; i++)
    {
        pthread_create(&thread_id[N + i], NULL, reader, &stuffs[i]);
    }

    for (int i = 0; i < N; i++)
    {
        sleep(distribution(generator));
        chrono::time_point<std::chrono::high_resolution_clock> end_time = std::chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end_time - start_time;
        osyncstream(outputFile) << "Student " << students[i] << " has arrived printing station at time " << (int)duration.count() << endl;
        pthread_create(&thread_id[i], NULL, student, &students[i]);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(thread_id[i], NULL);
    }

    for (int i = 0; i < 2; i++)
    {
        pthread_join(thread_id[N + i], NULL);
    }
    for (int i = 0; i < 4; i++)
    {
        sem_destroy(&S[i]);
    }
    for (int i = 0; i < G; i++)
    {
        sem_destroy(&GL[i]);
    }
    for (int i = 0; i < N; i++)
    {
        sem_destroy(&stu_S[i]);
    }
    sem_destroy(&bin_sem);
    sem_destroy(&B);
    sem_destroy(&mu);
    outputFile.close();
    return 0;
}