class Solution {
public:
    int quickSelect(vector<int>& a,int l,int r,int k){
      int q= randomPartition(a,l,r);
      if(q == k){return a[q];}
      else{
          return q<k? quickSelect(a,q+1,r,k):quickSelect(a,l,q-1,k);
      }  
    }
    int randomPartition(vector<int>& a,int l,int r){
        int i=rand()%(r-l+1)+l;
        swap(a[i],a[r]);
        int p=l-1;
        for(int j=l;j<r;j++){
            if(a[j]<=a[r]){
                swap(a[++p],a[j]);
            }
        }
        swap(a[p+1],a[r]);
        return p+1;
    }
    int findKthLargest(vector<int>& nums, int k) { 
        srand(time(0));
        return quickSelect(nums,0,nums.size()-1,nums.size()-k);
    }
};