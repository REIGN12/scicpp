#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<regex>
#include<cmath>
#include<algorithm>

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
        lc{a},rc{b},tag{FUNC}
        {func = opeff[_op];};

        double operator() (double x)
        {
            switch (tag)
            {
            case VAL:
                return this->val;
            case FUNC:
                return func(x,lc,rc);
            }
        }

    //private:
        double val;
        double(*func)(double,ExprNode*,ExprNode*);
        ExprNode* lc;
        ExprNode* rc;
        enum{VAL,FUNC};
        int tag;
        map<string,double(*)(double,ExprNode*,ExprNode*)> opeff
        {
            {"+",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)+(*b)(x);}},
            {"-",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)-(*b)(x);}},
            {"*",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)*(*b)(x);}},
            {"/",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)/(*b)(x);}},
            {"^",[](double x,ExprNode*a,ExprNode*b){return pow((*a)(x),(*b)(x));}}
        };
};

void func_parser_preprocesser(string& func_s)
{
    /* preprocess start */
    // 利用 regex 做预处理，使得表达式没有空白字符
    regex re_bl(R"(\s+)");
    func_s = regex_replace(func_s,re_bl,"");
    // 利用 regex 将表达式中的 operator 和 term 分开
    regex re_op(R"([\+\*\-\/\^])");// using raw string
    func_s = regex_replace(func_s,re_op," $0 ");
    // 利用 regex 将表达式中的 左右括号 分开
    regex re_lpar(R"(\()");
    func_s = regex_replace(func_s,re_lpar,"$0 ");
    regex re_rpar(R"(\))");
    func_s = regex_replace(func_s,re_rpar," $0");
}

ExprNode* func_parser(string func_s)
{
    // sep the op and term
    func_parser_preprocesser(func_s);
    
    // 返回 exprs (vec of string)
    auto end = sregex_token_iterator();
    regex re_bl(R"(\s+)");
    vector<string> exprs(sregex_token_iterator(func_s.begin(),func_s.end(),re_bl,-1),end);
    /* preprocess end */

    // 利用 map 表示运算符的优先级
    map<string,int> pri 
    {
        {"+",0},{"-",0},{"*",1},{"/",1},{"^",2}
    };
    regex re_op(R"([\+\-\*\/\^])");
    regex re_lpar(R"(\()");
    regex re_rpar(R"(\))");

    // 使用栈完成字符串转化为表达式
    vector<string> sop;
    vector<ExprNode*> sexpr;
    int sexpr_top = 0;
    for(auto&& s : exprs)
    {
        if(regex_match(s,re_op))
        {
            while(!sop.empty() && pri[sop.back()] > pri[s])
            {
                // 对于双目运算符
                sexpr_top-=2;
                sexpr.insert(sexpr.begin()+sexpr_top,
                new ExprNode{sop.back(),sexpr[sexpr_top],sexpr[sexpr_top+1]});
                sexpr_top++;
                sop.pop_back();
            }
            sop.push_back(s);
        }
        else if(regex_match(s,re_lpar))sop.push_back(s);
        else if(regex_match(s,re_rpar))
        {
            while(sop.back() != "(")
            {
                // 对于双目运算符
                sexpr_top-=2;
                sexpr.insert(sexpr.begin()+sexpr_top,
                new ExprNode{sop.back(),sexpr[sexpr_top],sexpr[sexpr_top+1]});
                sexpr_top++;
                sop.pop_back();
            }
            sop.pop_back();
        }
        else
        {
            sexpr.insert(sexpr.begin()+sexpr_top,
            "x"==s? new ExprNode() : new ExprNode(stod(s)));
            sexpr_top++;
        }
    }
    
    while(!sop.empty())
    {
        // 对于双目运算符
        sexpr_top -= 2;
        sexpr.insert(sexpr.begin()+sexpr_top,
        new ExprNode{sop.back(),sexpr[sexpr_top],sexpr[sexpr_top+1]});
        sexpr_top++;
        sop.pop_back();
    }
    return sexpr[0];

}

int main(int argc,char* argv[])
{
    ExprNode* p = func_parser(string(argv[1]));
    double x = stod(argv[2]);

    cout<<(*p)(x)<<"\n";
  
    return 0;
}