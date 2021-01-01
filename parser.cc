#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<regex>


using namespace std;

// 定义 expr tree 的节点(定义为 functor)
class ExprNode
{
    public:
        // for literal node
        ExprNode(double _val)
        :
        lc{nullptr},rc{nullptr},tag{VAL},val{_val}
        {};
        // for var node
        ExprNode()
        :
        lc{nullptr},rc{nullptr},tag{FUNC},
        func{[](double _x, ExprNode* a,ExprNode*b){return _x;}}
        {};
        // for binary operator node
        ExprNode(string _op,ExprNode*a,ExprNode*b)
        :
        op{_op},lc{a},rc{b},tag{FUNC}
        {func = opeff[_op];};

        double operator() (double x)
        {
            switch (tag)
            {
            case VAL:
                return val;
            case FUNC:
                return func(x,lc,rc);
            }
        }

    private:
        double val;
        double(*func)(double,ExprNode*,ExprNode*);
        ExprNode* lc;
        ExprNode* rc;
        string op;
        enum{VAL,FUNC};
        int tag;
        map<string,double(*)(double,ExprNode*,ExprNode*)> opeff
        {
            {"+",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)+(*b)(x);}},
            {"*",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)*(*b)(x);}}
        };
};

vector<ExprNode> func_parse(string func_s)
{
    //利用 regex 做预处理，使得表达式没有空白字符

    //利用 regex 做字符串分割, 分为 terms 和 ops 两部分
    regex re("[\\+\\*]");
    auto end = sregex_token_iterator();
    vector<string> terms(sregex_token_iterator(func_s.begin(),func_s.end(),re,-1),end);
    vector<string> ops(sregex_token_iterator(func_s.begin(),func_s.end(),re,0),end);

    // 使用栈完成字符串转化为表达式
    // 利用 map 表示运算符的优先级
    map<string,int> pri 
    {
        {"+",0},{"*",1}
    };

};



int main()
{
    ExprNode a;
    ExprNode b(10.);
    ExprNode c("+",&a,&b);

    double test = c(100.);

    cout << test;
    return 0;
}