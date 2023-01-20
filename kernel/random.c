// taken from
// https://www.cs.virginia.edu/~cr4bd/4414/F2018/files/lcg_parkmiller_c.txt


#define RANDOM_MAX ((1u << 31u) - 1u)

static unsigned random_seed = 1;

unsigned lcg_parkmiller(unsigned *state)
{
    const unsigned N = 0x7fffffff;
    const unsigned G = 48271u;

    unsigned div = *state / (N / G);  
    unsigned rem = *state % (N / G);  

    unsigned a = rem * G;        
    unsigned b = div * (N % G);  

    return *state = (a > b) ? (a - b) : (a + (N - b));
}

unsigned next_random() {
    return lcg_parkmiller(&random_seed);
}


