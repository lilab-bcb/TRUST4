#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <string>
#include <map>

#include "defs.h"

#define LINE_WIDTH 50000

char usage[] = "Usage: clone-evo xxx_cdr3.out [OPTIONS] > yyy_evo.out\n"
	"Options:\n" 
	"\t-a FLOAT: only consider clonotypes more abundant than specified value (default: 2.0)\n"
	;

struct _cdr3
{
	char *seq ;
	int clusterId ;
	int subId ;
	double abund ;
} ;

struct _adj
{
	int v ;
	int next ;
} ;


int HammingDistance( char *a, char *b )
{
	int i, ret = 0 ;
	for ( i = 0 ; a[i] ; ++i )
		if ( a[i] != b[i] )
			++ret ;
	return ret ;
}

void InsertAdj( int u, int v, struct _adj *adj, int &adjUsed )
{
	adj[adjUsed].v = v ;
	adj[adjUsed].next = adj[u].next ;
	adj[u].next = adjUsed ;
	++adjUsed ;
}

// Prim algorithm for minimum spanning tree
void Prim( int **dist, int n,  struct _adj *adj, int &adjUsed, int offset, std::vector<struct _cdr3> &cdr3s )
{
	int i, j ;
	struct _pair *minDist = new struct _pair[n] ; // a-dist, b-the node used to connect
	bool *used = new bool[n] ;
	memset( used, false, sizeof(bool) * n ) ;

	int max = -1 ;
	int maxTag = -1 ;
	for ( i = 0 ; i < n ; ++i )
		if ( cdr3s[i + offset].abund > max )
		{
			max = cdr3s[i + offset].abund ;
			maxTag = i ;
		}

	minDist[maxTag].a = 0 ;
	minDist[maxTag].b = 0 ;
	for ( i = 0 ; i < n ; ++i )
	{
		if ( i == maxTag )
			continue ;
		minDist[i].a = dist[maxTag][i] ;
		minDist[i].b = maxTag ;
	}
	used[maxTag] = true ;
	
	for ( i = 1 ; i < n ; ++i )
	{
		int min = LINE_WIDTH ;
		int minTag = -1 ;
		for ( j = 0 ; j < n ; ++j )
		{
			if ( !used[j] )
			{
				if ( minDist[j].a < min )
				{
					min = minDist[j].a ;
					minTag = j ;
				}
				else if ( minDist[j].a == min && cdr3s[j + offset].abund > cdr3s[minTag + offset].abund )
				{
					minTag = j ;
				}
			}
		}

		used[minTag] = true ;
		InsertAdj( minDist[minTag].b, minTag, adj, adjUsed ) ;
		InsertAdj( minTag, minDist[minTag].b , adj, adjUsed ) ;

		for ( j = 0 ; j < n ; ++j )
		{
			if ( !used[j] )
			{
				if ( dist[minTag][j] < minDist[j].a )
				{
					minDist[j].a = dist[minTag][j] ;
					minDist[j].b = minTag ;
				}
				else if ( dist[minTag][j] == minDist[j].a 
					&& cdr3s[minTag + offset].abund > cdr3s[ minDist[j].b + offset ].abund )
				{
					minDist[j].b = minTag ;
				}
			}
		}
	}

	delete[] used ;
	delete[] minDist ;
}

// Return: whether it is a leaf.
int LongestRootLeafDistMST( int tag, struct _adj *adj, int n, bool *visited, int **dist )
{
	visited[tag] = true ;

	int p = adj[tag].next ;
	int ret = 0 ;
	while ( p != -1 )
	{
		if ( visited[ adj[p].v ] == false )
		{
			int tmp = dist[tag][ adj[p].v ] + LongestRootLeafDistMST( adj[p].v, adj, n, visited, dist ) ;
			if ( tmp > ret )
				ret = tmp ;
		}
		p = adj[p].next ;
	}
	return ret ;
}

void PrintMST( FILE *fp, int tag, int parent, bool *visited, int **dist, int offset, struct _adj *adj, 
	std::vector<struct _cdr3> &cdr3, std::vector<std::string> clusterIdToName )
{
	visited[tag] = true ;

	int p = adj[tag].next ;
	int childCnt = 0 ;
	while ( p != -1 )
	{
		if ( visited[adj[p].v] == false )
		{
			if ( childCnt == 0 )
				fprintf( fp, "(" ) ;
			else
				fprintf( fp, "," ) ;
			PrintMST( fp, adj[p].v, tag, visited, dist, offset, adj, cdr3, clusterIdToName ) ;
			++childCnt ;
		}
		p = adj[p].next ;
	}
	if ( childCnt > 0 )
		fprintf( fp, ")" ) ;
	if ( parent == tag ) // root
		fprintf( fp, "%s_%d_%.2lf;\n", clusterIdToName[ cdr3[tag + offset].clusterId ].c_str(), cdr3[tag + offset].subId,
			cdr3[tag + offset].abund ) ; 
	else
		fprintf( fp, "%s_%d_%.2lf:%d", clusterIdToName[ cdr3[tag + offset].clusterId ].c_str(), 
			cdr3[tag + offset].subId, cdr3[tag + offset].abund, dist[parent][tag] ) ;
}


char buffer[LINE_WIDTH] ;
char buffer2[LINE_WIDTH] ;
char seq[LINE_WIDTH] ;

std::map<std::string, int> clusterNameToId ;
std::vector<std::string> clusterIdToName ;

int main( int argc, char *argv[] )
{
	if ( argc <= 1 )
	{
		fprintf( stderr, "%s", usage ) ;
		return 0 ;
	}

	int i, j, k ;

	double minAbund = 2 ;

	for ( i = 2 ; i < argc ; ++i )
	{
		if ( !strcmp( argv[i], "-a" ) )
		{
			minAbund = atof( argv[i + 1] ) ;
			i += 1 ;
		}
	}

	FILE *fp ;
	fp = fopen( argv[1], "r" ) ;

	std::vector<struct _cdr3> cdr3s ;
	while ( fgets( buffer, sizeof(buffer), fp ) != NULL )
	{
		char clusterIdBuffer[1024]  ;
		double score, abund ;
		sscanf( buffer, "%s %d %s %s %s %s %s %s %s %lf %lf", clusterIdBuffer, &k, 
			buffer2, buffer2, buffer2, buffer2,
			buffer2, buffer2, seq,
			&score, &abund ) ;

		if ( score == 0 )
			continue ;

		struct _cdr3 nc ;
		nc.seq = strdup( seq ) ;
		int clusterId ;
		std::string clusterName( clusterIdBuffer ) ;
		if ( clusterNameToId.find( clusterName ) != clusterNameToId.end() )
		{
			clusterId = clusterNameToId[clusterName] ;
		}
		else
		{
			clusterId = clusterIdToName.size() ;
			clusterIdToName.push_back( clusterName ) ;
			clusterNameToId[ clusterName ] = clusterId ;
		}
		nc.clusterId = clusterId ;
		nc.subId = k ;
		nc.abund = abund ;

		if ( nc.abund < minAbund )
			continue ;
		cdr3s.push_back( nc ) ;
	}
	
	// Process cluster by cluster. Assume the input is already sorted by cluster
	int cdr3Cnt = cdr3s.size() ;
	for ( i = 0 ; i < cdr3Cnt ; )
	{
		for ( j = i + 1 ; j < cdr3Cnt ; ++j )
			if ( cdr3s[j].clusterId != cdr3s[i].clusterId )
				break ;
		int n = j - i ;
		if ( n <= 2 )
		{
			i = j ;
			continue ;
		}
		int l ;
		int **dist = new int*[n] ;
		for ( k = 0 ; k < n ; ++k )
			dist[k] = new int[n] ;
		struct _adj *adj = new struct _adj[3 * n] ;
		int adjUsed = 0 ;
		
		// Build the distance matrix
		for ( k = 0 ; k < n ; ++k )
		{
			dist[k][k] = 0 ;
			for ( l = k + 1 ; l < n ; ++l )
				dist[k][l] = dist[l][k] = HammingDistance( cdr3s[k + i].seq, cdr3s[l + i].seq ) ;
			adj[k].next = -1 ;
		}
		adjUsed = k ;
		
		// Build the tree
		Prim( dist, n, adj, adjUsed, i, cdr3s ) ;

		// Find the root for the tree
		std::vector<int> leaves ;
		for ( k = 0 ; k < n ; ++k )
		{
			if ( adj[ adj[k].next ].next == -1 ) // The algorithm should make sure adj[k].next is not -1
				leaves.push_back( k ) ;
		}

		int bestRoot = -1 ;
		int minMaxRootLeafDist = LINE_WIDTH ;
		bool *visited = new bool[n] ;
		for ( k = 0 ; k < n ; ++k )
		{
			memset( visited, false, sizeof( bool ) * n ) ;
			int max = LongestRootLeafDistMST( k, adj, n, visited, dist ) ;
			if ( max < minMaxRootLeafDist )
			{
				bestRoot = k ;
				minMaxRootLeafDist = max ;
			}
			else if ( max == minMaxRootLeafDist )
			{
				if ( cdr3s[k + i].abund > cdr3s[bestRoot +i].abund )
					bestRoot = k ;
			}
		}

		// Output the tree
		memset( visited, false, sizeof( bool ) * n ) ;
		PrintMST( stdout, bestRoot, bestRoot, visited, dist, i, adj, cdr3s, clusterIdToName ) ;
	
		delete[] visited ;
		delete[] adj ;
		for ( k = 0 ; k < n ; ++k )
			delete[] dist[k] ;
		delete[] dist ;

		i = j ;
		continue ;
	}
	fclose( fp ) ;
	
	for ( i = 0 ; i < cdr3Cnt ; ++i )
		free( cdr3s[i].seq ) ;
	return 0 ;
}
