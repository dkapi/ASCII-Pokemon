#define TILECNT 10
struct TreeTile
{
   const char* name ="tree";
   const char ascii = '^';
   const float ajacency[TILECNT];
   const int id = 94;
};

struct mountainTile
{
   const char* name = "mountain";
   const char ascii = '%';
   const float ajacency[TILECNT];
   const int id = 37;
};

struct tallGrass
{
   const char* name = "tall-grass";
   const char ascii = ':';
   const float ajacency[TILECNT];
   const int id = 58;
};

struct clearing
{
   const char* name = "clearing";
   const char ascii = '.';
   const float adjacency[TILECNT];
    const int id = 46;
