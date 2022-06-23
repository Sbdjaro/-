#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
	int len;
	int* to;
} Node;

int readnode(int col, Node* points)
{
	int x = -2;
	int i, y, k, j;
	char c;
	for (i = 0; i < col; i++)
	{
		scanf("%c", &c);
		j = 0;
		while (1) {
			k = c - '0';
			scanf("%c", &c);
			while (c != ' ' && c != '\n')
			{
				k *= 10;
				k += c - '0';
				scanf("%c", &c);
			}
			if (c == '\n' && j == 0)
			{
				points[i].to = (int*)calloc(1, sizeof(int));
				points[i].to[j] = k - 1;
				j += 1;
				break;
			}
			if (c == '\n') {
				points[i].to[j] = k - 1;
				j += 1;
				break;
			}
			if (j == 0)
				points[i].to = (int*)calloc(col, sizeof(int));
			points[i].to[j] = k - 1;
			j += 1;
			scanf("%c", &c);
		}
		if (k == 0)
			points[i].len = 0;
		else
			points[i].len = j;
		if (points[i].len > 1)
			x = i;
	}
	return x;
}

typedef struct vector
{
	int size;
	int* arr;
} vector;

vector* createVector(int size)
{
	vector* v = (vector*)malloc(sizeof(vector));
	v->arr = (int*)malloc(size * sizeof(int));
	for (int i = 0; i < size; i++)
	{
		v->arr[i] = -1;
	}
	v->size = -1;
	return v;
}

void find_MinWay(int col, int x, int* used, Node* points, vector* path)
{
	int k = 1;
	while (1)
	{
		int s = 0;
		for (int i = 0; i < col; i++)
		{
			if (used[i] == k - 1)
			{
				for (int j = 0; j < points[i].len; j++) {
					if (points[i].to[j] == -1)
						continue;
					if (used[points[i].to[j]] == -1)
					{
						used[points[i].to[j]] = k;
						s += 1;
						if (i == x)
						{
							path->arr[k - 1] = i;
							path->size = k;
						}
						else if (path->size == -1)
						{
							path->arr[k - 1] = i;
						}
					}
				}
			}
		}

		k += 1;
		if (s == 0)
			break;
	}
}

void printvec(vector* vec) {
	for (int i = 0; i < vec->size; i++)
		printf("%d ", vec->arr[i] + 1);
}

void print_way(int x, vector* XXX_to_x, int* visited, int finish, Node* points) {
	int i, j, k;
	if (XXX_to_x->size == -1 || x == finish)
	{
		for (i = 0; i < visited[finish]; i++)
		{
			printf("%d ", XXX_to_x->arr[i] + 1);
		}
		printf("%d\n", finish + 1);
	}
	else
	{
		vector* x_to = createVector(visited[finish] - XXX_to_x->size + 1);
		for (i = 0; i < points[x].len; i++)
		{
			for (j = 0; j < visited[finish] - XXX_to_x->size + 1; j++)
				x_to->arr[j] = -1;
			k = points[x].to[i];
			for (j = 0; j < visited[finish] - XXX_to_x->size + 1; j++)
			{
				x_to->arr[j] = k;
				if (k == x || points[k].to[0] == -1)
					break;
				k = points[k].to[0];
			}

			if (x_to->arr[visited[finish] - XXX_to_x->size] == finish) {
				x_to->size = visited[finish] - XXX_to_x->size + 1;
				printvec(XXX_to_x);
				printvec(x_to);
				printf("\n");
			}
		}
		free(x_to->arr);
		free(x_to);
	}
}


int main()
{
	int col, first, last;
	int i;

	scanf("%d %d %d\n", &col, &first, &last);

	first -= 1;
	last -= 1;

	int* visited = (int*)calloc(col, sizeof(int));
	int* visited2 = (int*)calloc(col, sizeof(int));

	Node* points = (Node*)calloc(col, sizeof(Node));
	for (i = 0; i < col; i++)
	{
		visited[i] = -1;
		visited2[i] = -1;
	}
	int x = readnode(col, points);
	vector* first_to_x = createVector(col);
	vector* last_to_x = createVector(col);

	visited[first] = 0;
	visited2[last] = 0;

	find_MinWay(col, x, visited, points, first_to_x);
	find_MinWay(col, x, visited2, points, last_to_x);


	if (visited[last] == -1 && visited2[first] == -1)
	{
		printf("-1\n");
	}
	else {
		if (((visited[last] < visited2[first]) && visited[last]!=-1)|| visited2[first]==-1)
		{
			if (x == -2)
				x = last;
			printf("%d\n", visited[last]);
			print_way(x, first_to_x, visited, last, points);

		}

		if (((visited[last] > visited2[first])&& visited2[first]!=-1) || visited[last] == -1)
		{
			if (x == -2)
				x = first;
			printf("%d\n", visited2[first]);
			print_way(x, last_to_x, visited2, first, points);
		}

		if (visited[last] == visited2[first])
		{
			printf("%d\n", visited[last]);
			if (x == -2) {
				print_way(last, first_to_x, visited, last, points);
				print_way(first, last_to_x, visited2, first, points);
			}
			else {
				print_way(x, first_to_x, visited, last, points);
				print_way(x, last_to_x, visited2, first, points);
			}
		}
	}
	free(first_to_x->arr);
	free(last_to_x->arr);
	free(first_to_x);
	free(last_to_x);
	for (i = 0; i < col; i++)
		free(points[i].to);
	free(points);
	free(visited);
	free(visited2);
	return(0);
}