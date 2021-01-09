#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<regex>
#include<cmath>
#include<algorithm>

using namespace std;

void remove_blank(string& s)
{
    // 利用 regex 做预处理，使得表达式没有空白字符
    regex re_bl(R"(\s+)");
    s = regex_replace(s,re_bl,"");
}

vector<string> split(string s, regex re)
{
    auto end = sregex_token_iterator();
    vector<string> exprs(sregex_token_iterator(s.begin(),s.end(),re,-1),end);
    return exprs;
}

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
        // for unary operator node
        ExprNode(string _op,ExprNode*a)
        :
        lc{a},rc{nullptr},tag{FUNC}
        {func = opeff[_op];};
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

    private:
        double val;
        double(*func)(double,ExprNode*,ExprNode*);
        ExprNode* lc;
        ExprNode* rc;
        enum{VAL,FUNC};
        int tag;
        // use opeff as static(owned by all member in the class)
        static map<string,double(*)(double,ExprNode*,ExprNode*)> opeff;
        
};
map<string,double(*)(double,ExprNode*,ExprNode*)> ExprNode::opeff
{
            {"+",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)+(*b)(x);}},
            {"-",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)-(*b)(x);}},
            {"*",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)*(*b)(x);}},
            {"/",[](double x,ExprNode*a,ExprNode*b){return (*a)(x)/(*b)(x);}},
            {"^",[](double x,ExprNode*a,ExprNode*b){return pow((*a)(x),(*b)(x));}},
            {"sin",[](double x, ExprNode*a,ExprNode*b){return sin((*a)(x));}},
            {"cos",[](double x, ExprNode*a,ExprNode*b){return cos((*a)(x));}},
            {"tan",[](double x, ExprNode*a,ExprNode*b){return tan((*a)(x));}},
            {"exp",[](double x, ExprNode*a,ExprNode*b){return exp((*a)(x));}},
            {"log",[](double x, ExprNode*a,ExprNode*b){return log((*a)(x));}},
            {"gamma",[](double x, ExprNode*a,ExprNode*b){return tgamma((*a)(x));}}
};

void func_parser_preprocesser(string& func_s)
{
    /* preprocess start */
    // 首先去除空白
    remove_blank(func_s);
    // 利用 regex 将表达式中的 operator 和 term 分开
    // 分出 binary op
    regex re_biop(R"([\+\*\-\/\^])");// using raw string
    func_s = regex_replace(func_s,re_biop," $0 ");
    // 分出 unary op
    regex re_unop(R"((sin)|(cos)|(tan)|(exp)|(log)|(gamma))");
    func_s = regex_replace(func_s,re_unop,"$0 ");
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
    regex re_bl(R"(\s+)");
    vector<string> exprs = split(func_s,re_bl);
    /* preprocess end */

    // 利用 map 表示运算符的优先级
    map<string,int> pri 
    {
        {"+",0},{"-",0},{"*",1},{"/",1},{"^",2},
        {"sin",4},{"cos",4},{"tan",4},
        {"exp",4},{"log",4},
        {"gamma",4}
    };
    regex re_op(R"([\+\-\*\/\^]|(sin)|(cos)|(tan)|(exp)|(log)|(gamma))");
    regex re_lpar(R"(\()");
    regex re_rpar(R"(\))");
    regex re_biop(R"([\+\-\*\/\^])");


    // 使用栈完成字符串转化为表达式
    vector<string> sop;
    vector<ExprNode*> sexpr;
    int sexpr_top = 0;
    auto un_op_pop = [&sexpr,&sexpr_top,&sop]()
    {
        sexpr_top--;
        sexpr.insert(sexpr.begin()+sexpr_top,
        new ExprNode(sop.back(),sexpr[sexpr_top]));
        sexpr_top++;
        sop.pop_back();
    };
    auto bin_op_pop = [&sexpr,&sexpr_top,&sop]()
    {
        sexpr_top-=2;
        sexpr.insert(sexpr.begin()+sexpr_top,
        new ExprNode{sop.back(),sexpr[sexpr_top],sexpr[sexpr_top+1]});
        sexpr_top++;
        sop.pop_back();
    };
    for(auto&& s : exprs)
    {
        if(regex_match(s,re_op))
        {
            while(!sop.empty() && pri[sop.back()] > pri[s])
            {
                // 对于双目运算符
                if(regex_match(sop.back(),re_biop))bin_op_pop();
                else un_op_pop();
            }
            sop.push_back(s);
        }
        else if(regex_match(s,re_lpar))sop.push_back(s);
        else if(regex_match(s,re_rpar))
        {
            while(sop.back() != "(")
            {
                // 对于双目运算符
                if(regex_match(sop.back(),re_biop))bin_op_pop();
                else un_op_pop();
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
        if(regex_match(sop.back(),re_biop))bin_op_pop();
        else un_op_pop();
    }
    return sexpr[0];

}

double integrate(ExprNode* f, double a, double b)
{
    // Use Simpson method to cal integration

    const int num = 100; // num should be even
    double res = (*f)(a)+(*f)(b);
    const double h = (b-a)/num;
    double x;
    for(int i = 1,flag = 1; i < num; ++i,flag = -flag)
    {
        x = a+h*i;
        if(flag == 1)res += 4*(*f)(x);
        else res += 2*(*f)(x);
    }
    return res / 3 * h;
}

double diff(ExprNode* f,double x)
{
    // use mid point method to cal derivative
    const double eps = 1e-4;
    return ((*f)(x+eps)-(*f)(x-eps)) / (2*eps);
}

double solve(ExprNode* f, double x0)
{
    // Try Newton Method 
    const int m_cnt = 10000;
    int cnt = 0;
    const double eps = 1e-5;
    double x = x0;
    double chan;
    do
    {
        chan = (*f)(x)/diff(f,x);
        x -= chan;
        cnt++;
        if(cnt>m_cnt)
        {
            cout<<"Wrong Input!"<<"\n";
            return 1;
        }
    } while (chan*chan > eps*eps);
    return x;
}


double statement_parser(string statement)
{
    enum{INTEGRATE,DIFF,SOLVE};
    int tag;
    if(regex_search(statement,regex(R"(integrate)"))) tag = INTEGRATE;
    else if(regex_search(statement,regex(R"(diff)"))) tag = DIFF;
    else if(regex_search(statement,regex(R"(solve)"))) tag = SOLVE;

    remove_blank(statement);
    regex re_bound(R"((integrate\[)|(diff\[)|(solve\[)|(\]))");
    statement = regex_replace(statement,re_bound,"");
    regex re_sep(R"(,)");
    vector<string> exprs = split(statement,re_sep);
    
    switch (tag)
    {
    case INTEGRATE:
        return integrate(func_parser(exprs[0]),stod(exprs[1]),stod(exprs[2]));
    case DIFF:
        return diff(func_parser(exprs[0]),stod(exprs[1]));
    case SOLVE:
        return solve(func_parser(exprs[0]),stod(exprs[1]));
    default:
        cout<<"Not a Statement!\n";
        return 0;
        break;
    }
    return 0;
}

int main(int argc,char* argv[])
{
    cout<<statement_parser(string(argv[1]))<<"\n";
    //ExprNode* p = func_parser(string(argv[1]));
    //double x0 = stod(argv[2]);
    //cout<<integrate(p,0,1)<<"\n";
    //cout<<solve(p,x0)<<"\n";
    //cout<<diff(p,x)<<"\n";
  
    return 0;
}