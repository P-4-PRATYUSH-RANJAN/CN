#include <bits/stdc++.h>
using namespace std;

int main() {
   vector<int> vec1, vec2;
   int code, CRC;
   cout<<"Enter the code and CRC: ";
   cin>>code>>CRC;
   while(code){
    vec1.push_back(code%10);
    code=code/10;
   }
   while(CRC){
    vec2.push_back(CRC%10);
    CRC=CRC/10;
   }
   
   reverse(vec1.begin(), vec1.end());
   reverse(vec2.begin(), vec2.end());
   int m = vec2.size();
   vector<int> vec3(m, 0);
   for(int i=0; i<m-1; i++){
    vec1.push_back(0);
   }
   
   int n = vec1.size();
     
//   for(int i=0; i<n; i++){
//   cout<<vec1[i];
//   }
//   cout<<endl;
//   for(int j=0; j<m; j++){
//   cout<<vec2[j];
//   }
   
   for(int i=0; i<n-m+1; i++){
    if(vec1[i]==1){
    for(int j=0; j<m; j++){
    vec1[i+j] = (vec1[j+i]^vec2[j]);
//   cout<<vec1[i+j]<<" ";
  }
  //cout<<endl;
}
   }
   
   for(int i=n-m+1; i<n; i++){
    cout<<vec1[i];
   }
   
   return 0;
