#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct MatchResult { bool found; size_t pos; };

MatchResult brute_force(const string& t, const string& p){
    if(p.empty()) return {true,0};
    if(p.size()>t.size()) return {false,string::npos};
    for(size_t i=0;i+p.size()<=t.size();++i){
        size_t j=0;
        while(j<p.size() && t[i+j]==p[j]) ++j;
        if(j==p.size()) return {true,i};
    }
    return {false,string::npos};
}

vector<int> kmp_pi(const string& p){
    vector<int> pi(p.size());
    for(size_t i=1;i<p.size();++i){
        int j=pi[i-1];
        while(j>0 && p[i]!=p[j]) j=pi[j-1];
        if(p[i]==p[j]) ++j;
        pi[i]=j;
    }
    return pi;
}

MatchResult kmp(const string& t,const string& p){
    if(p.empty()) return {true,0};
    auto pi=kmp_pi(p);
    int j=0;
    for(size_t i=0;i<t.size();++i){
        while(j>0 && t[i]!=p[j]) j=pi[j-1];
        if(t[i]==p[j]) ++j;
        if(j==(int)p.size()) return {true, i+1-p.size()};
    }
    return {false,string::npos};
}

MatchResult sunday(const string& t,const string& p){
    if(p.empty()) return {true,0};
    if(p.size()>t.size()) return {false,string::npos};
    array<int,256> shift; shift.fill((int)p.size()+1);
    for(size_t i=0;i<p.size();++i) shift[(unsigned char)p[i]]=(int)p.size()-i;
    size_t i=0;
    while(i+p.size()<=t.size()){
        size_t j=0;
        while(j<p.size() && t[i+j]==p[j]) ++j;
        if(j==p.size()) return {true,i};
        if(i+p.size()>=t.size()) break;
        i += shift[(unsigned char)t[i+p.size()]];
    }
    return {false,string::npos};
}

MatchResult binary_sunday(const string& t,const string& p){
    if(p.empty()) return {true,0};
    vector<int> shift(2, (int)p.size()+1);
    for(size_t i=0;i<p.size();++i) shift[p[i]-'0']=(int)p.size()-i;
    size_t i=0;
    while(i+p.size()<=t.size()){
        size_t j=0;
        while(j<p.size() && t[i+j]==p[j]) ++j;
        if(j==p.size()) return {true,i};
        if(i+p.size()>=t.size()) break;
        i += shift[t[i+p.size()]-'0'];
    }
    return {false,string::npos};
}

vector<int> z_function(const string& s){
    int n=s.size(); vector<int> z(n);
    int l=0,r=0;
    for(int i=1;i<n;++i){
        if(i<=r) z[i]=min(r-i+1,z[i-l]);
        while(i+z[i]<n && s[z[i]]==s[i+z[i]]) ++z[i];
        if(i+z[i]-1>r){l=i;r=i+z[i]-1;}
    }
    return z;
}

MatchResult gusfield_z(const string& t,const string& p){
    if(p.empty()) return {true,0};
    string s=p+"$"+t;
    auto z=z_function(s);
    for(size_t i=p.size()+1;i<s.size();++i)
        if((size_t)z[i]>=p.size()) return {true,i-p.size()-1};
    return {false,string::npos};
}

MatchResult rabin_karp(const string& t,const string& p){
    if(p.empty()) return {true,0};
    if(p.size()>t.size()) return {false,string::npos};
    const uint64_t B=911382323ull;
    uint64_t ph=0, th=0, pow=1;
    for(size_t i=0;i<p.size();++i){
        ph=ph*B + (unsigned char)p[i]+1;
        th=th*B + (unsigned char)t[i]+1;
        if(i+1<p.size()) pow*=B;
    }
    for(size_t i=0;i+p.size()<=t.size();++i){
        if(ph==th && t.compare(i,p.size(),p)==0) return {true,i};
        if(i+p.size()<t.size()){
            th -= ((unsigned char)t[i]+1)*pow;
            th = th*B + (unsigned char)t[i+p.size()]+1;
        }
    }
    return {false,string::npos};
}

MatchResult fsm(const string& t,const string& p){
    if(p.empty()) return {true,0};
    vector<array<int,256>> trans(p.size()+1);
    for(size_t q=0;q<=p.size();++q){
        for(int c=0;c<256;++c){
            size_t k=min(p.size(), q+1);
            string pref = p.substr(0,q) + char(c);
            while(k>0){
                if(pref.size()>=k && pref.substr(pref.size()-k)==p.substr(0,k)) break;
                --k;
            }
            trans[q][c]=k;
        }
    }
    size_t state=0;
    for(size_t i=0;i<t.size();++i){
        state = trans[state][(unsigned char)t[i]];
        if(state==p.size()) return {true,i+1-p.size()};
    }
    return {false,string::npos};
}

// Wildcards
bool wild_match(const string& s,const string& p){
    size_t n=s.size(), m=p.size();
    vector<vector<char>> dp(n+1, vector<char>(m+1,false));
    dp[0][0]=true;
    for(size_t j=1;j<=m;++j){
        if(p[j-1]=='*' && (j<2 || p[j-2]!='\\')) dp[0][j]=dp[0][j-1];
    }
    auto isEscaped=[&](size_t j){ return j>0 && p[j-1]=='\\'; };
    for(size_t i=1;i<=n;++i){
        for(size_t j=1;j<=m;++j){
            char pc=p[j-1];
            if(pc=='*' && !isEscaped(j-1)) dp[i][j]=dp[i][j-1]||dp[i-1][j];
            else if(pc=='?' && !isEscaped(j-1)) dp[i][j]=dp[i-1][j-1];
            else {
                char lit=pc;
                if(pc!='\\' && isEscaped(j-1)) lit=pc;
                dp[i][j]=dp[i-1][j-1] && s[i-1]==lit;
            }
        }
    }
    return dp[n][m];
}

struct BenchRow{string algo; size_t n; double ms;};

template<class F>
double time_ms(F f, int reps=3){
    double best=1e100;
    for(int i=0;i<reps;++i){
        auto st=chrono::high_resolution_clock::now();
        f();
        auto en=chrono::high_resolution_clock::now();
        best=min(best, chrono::duration<double,milli>(en-st).count());
    }
    return best;
}

int main(){
    mt19937 rng(42);
    string alphabet="abcdefghijklmnopqrstuvwxyz ";
    auto gen=[&](size_t n){ string s; s.reserve(n); uniform_int_distribution<int>d(0,alphabet.size()-1); for(size_t i=0;i<n;++i)s.push_back(alphabet[d(rng)]); return s;};
    vector<size_t> lengths={10000,20000,50000,100000,200000};
    string pattern_small="new computer";
    string pattern_large=gen(500);

    vector<pair<string,function<MatchResult(const string&,const string&)>>> algos={
        {"BruteForce",brute_force},{"Sunday",sunday},{"KMP",kmp},{"FSM",fsm},{"RabinKarp",rabin_karp},{"GusfieldZ",gusfield_z}
    };

    ofstream csv("benchmark.csv");
    csv<<"algo,text_len,pattern,ms\n";
    for(auto n:lengths){
        string t=gen(n);
        t.replace(n/2, pattern_small.size(), pattern_small);
        t.replace(n/3, min(pattern_large.size(), n/3), pattern_large.substr(0,min(pattern_large.size(),n/3)));
        for(auto &a:algos){
            double s=time_ms([&]{ a.second(t,pattern_small); });
            csv<<a.first<<","<<n<<",small,"<<fixed<<setprecision(4)<<s<<"\n";
            double l=time_ms([&]{ a.second(t,pattern_large); },1);
            csv<<a.first<<","<<n<<",large,"<<fixed<<setprecision(4)<<l<<"\n";
        }
    }

    // Wacky races constructions
    string T1(120000,'0'); T1[119999]='1'; string P1="0000000000000000001"; // binary sunday win
    string T2(120000,'a'); T2[119999]='b'; string P2(200,'a'); P2.back()='b';
    string T3(120000,'a'); string P3="aaaaaaaaaaaaaaaaab";

    auto bs=time_ms([&]{binary_sunday(T1,P1);},5);
    auto gz=time_ms([&]{gusfield_z(T1,P1);},5);
    auto k=time_ms([&]{kmp(T2,P2);},5);
    auto rk=time_ms([&]{rabin_karp(T2,P2);},5);
    auto rk2=time_ms([&]{rabin_karp(T3,P3);},5);
    auto su=time_ms([&]{sunday(T3,P3);},5);

    ofstream wr("wacky_races.txt");
    wr<<"BinarySunday="<<bs<<" GusfieldZ="<<gz<<" ratio="<<(gz/bs)<<"\n";
    wr<<"KMP="<<k<<" RabinKarp="<<rk<<" ratio="<<(rk/k)<<"\n";
    wr<<"RabinKarp="<<rk2<<" Sunday="<<su<<" ratio="<<(su/rk2)<<"\n";

    cout<<"Generated benchmark.csv and wacky_races.txt\n";
    cout<<"Wildcard example: "<<wild_match("abcde","a*de")<<"\n";
    return 0;
}
