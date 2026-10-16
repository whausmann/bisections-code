"""Compare production DFS with independent balanced-subset enumeration."""
from pathlib import Path
import os, subprocess, tempfile
root = Path(__file__).resolve().parents[1]
harness = r'''
#define main production_main
#include "bisection_checker.cpp"
#undef main
#include <random>
Result brute(const Graph& g) {
 Result r;
 for (U64 mask=1; mask<(U64{1}<<g.n); mask+=2) {
  if(std::popcount(mask)!=g.n/2) continue;
  bool ok=true; int l=0, rr=0;
  for(int v=0;v<g.n;++v) {
   bool side=(mask>>v)&1; int cross=0;
   for(int u=0;u<g.n;++u) if((g.adj[v]>>u)&1) cross+=(((mask>>u)&1)!=side);
   if(cross<2) {ok=false;break;}
   if(cross==3) (side?l:rr)++;
  }
  if(ok) {r.external=true; if(l>=4 && rr>=4) {r.sufficient=true;break;}}
 }
 return r;
}
int main() {
 std::mt19937 rng(20261016); int tested=0, success=0, failure=0;
 for(int n: {4,6,8,10,12,14,16}) {
  int accepted=0;
  for(int attempt=0;attempt<100000 && accepted<40;++attempt) {
   std::vector<int> stubs; for(int v=0;v<n;++v) for(int d=0;d<3;++d) stubs.push_back(v);
   std::shuffle(stubs.begin(),stubs.end(),rng); Graph g;g.n=n;bool ok=true;
   for(int i=0;i<3*n;i+=2) {int a=stubs[i],b=stubs[i+1]; if(a==b || ((g.adj[a]>>b)&1)) {ok=false;break;}g.add(a,b);}
   if(!ok || !connected(g)) continue;
   ++accepted; auto a=Search(g).run(),b=brute(g);
   if(a.external!=b.external || a.sufficient!=b.sufficient) return 1;
   ++tested; if(a.sufficient) ++success; else ++failure;
   if(encode(decode(encode(g)))!=encode(g)) return 2;
  }
  if(accepted!=40) return 3;
 }
 std::cout<<"PASS: "<<tested<<" connected simple cubic graphs (orders 4–16), independent exhaustive balanced-subset comparison; "<<success<<" sufficient, "<<failure<<" insufficient.\n";
}
'''
with tempfile.TemporaryDirectory() as temp:
    t=Path(temp); (t/'test.cpp').write_text(harness)
    subprocess.run([os.environ.get('CXX','c++'),'-std=c++20','-O2','-I',str(root),str(t/'test.cpp'),'-o',str(t/'test')],check=True)
    subprocess.run([str(t/'test')],check=True)
print('The recorded 36-vertex run was not repeated.')
