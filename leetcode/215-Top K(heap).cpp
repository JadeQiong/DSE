class Solution {
public:
    void adjust(int id,vector<int>& h,int siz){
        int l =id*2+1;
        int r= id*2+2;
        int maxId = id;
        if(l<siz && h[l]>h[maxId]){
            maxId = l;
        }
        if(r<siz && h[r]>h[maxId]){
            maxId = r;
        }
        //cout<<maxId<<" "<<siz<<endl;
        if(maxId!=id){
           // cout<<maxId<<" "<<id<<endl;
            swap(h[maxId],h[id]);
            adjust(maxId,h,siz);
        }
    }
    void buildHeap(vector<int>& h,int siz){
        for(int i=siz/2;i>=0;i--){
            adjust(i,h,siz);
        }
    }
    int findKthLargest(vector<int>& nums, int k) {
        int siz = nums.size();
        buildHeap(nums, siz);
        for(int i=nums.size()-1;i>=nums.size()-k+1;i--){
            swap(nums[0],nums[i]);
            siz--;
            adjust(0, nums,siz);
        }
        return nums[0];    
    }
};