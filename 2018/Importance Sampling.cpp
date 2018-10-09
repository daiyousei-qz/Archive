#include <chrono>
#include <random>
#include <memory>
#include <algorithm>
#include <functional>
#include <numeric>

// A demostration of Monte Carlo Integration with/without importance sampling
// int x^2 dx from 0 to 5

using namespace std;

template<typename X, typename F, typename PDF>
double integrate(int n, X& sample_x, const F& f, const PDF& pdf)
{
    static auto rnd=default_random_engine{ random_device{}() };

    double result=0;
    for (int i=0; i < n; ++i)
    {
        auto xi=sample_x(rnd);
        result+=f(xi) / pdf(xi);
    }

    return result / n;
}

static auto f=[](double x) {return x * x; };

double integrate_importance_sampling(int n)
{
    double i[]={ 0,5 };
    double w[]={ 0,1 };
    auto dist=piecewise_linear_distribution<>{ begin(i), end(i), begin(w) };

    return integrate(n, dist, f, [](double x) { return x / 12.5; });
}

double integrate_uniform_sampling(int n)
{
    auto dist=uniform_real_distribution<>{ 0., 5. };

    return integrate(n, dist, f, [](double x) { return .2; });
}

tuple<double, double, double> calc_avgstderr(const vector<double> estimates, double expected)
{
    double n=estimates.size();
    double avg=accumulate(estimates.begin(), estimates.end(), 0.)/n;
    
    double std=0.;
    double err=0.;
    for (auto x : estimates)
    {
        std+=pow(x - avg, 2);
        err+=pow(x - expected, 2);
    }

    std=sqrt(std) /n;
    err=sqrt(err) /n;

    return { avg, std, err };
}

int main()
{
    constexpr int sample=100;
    constexpr int test=100;

    auto rnd=default_random_engine{ random_device{}() };

    auto expected=125. / 3.;

    // importance sampling(linear)
    printf("[importance sampling]\n");
    for (auto sample=1; sample < 20; sample+=1)
    {
        auto estimates=vector<double>{};
        generate_n(back_inserter(estimates), test, bind(integrate_importance_sampling, sample));
        
        auto[avg, std, err]=calc_avgstderr(estimates, expected);
        printf("expect:%lf avg:%lf std:%lf err:%lf\n", expected, avg, std, err);
        //printf("%lf\n", err/expected);
    }

    // uniform sampling
    printf("[uniform sampling]\n");
    for (auto sample=1; sample < 20; sample+=1)
    {
        auto estimates=vector<double>{};
        generate_n(back_inserter(estimates), test, bind(integrate_uniform_sampling, sample));

        auto[avg, std, err]=calc_avgstderr(estimates, expected);
        printf("expect:%lf avg:%lf std:%lf err:%lf\n", expected, avg, std, err);
        //printf("%lf\n", err/expected);
    }

    system("pause");
    return 0;
}