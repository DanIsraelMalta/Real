[![Codacy Badge](https://api.codacy.com/project/badge/Grade/b81b4d5ee5414219ac6444dc766c3b4b)](https://www.codacy.com/app/DanIsraelMalta/Real?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=DanIsraelMalta/Real&amp;utm_campaign=Badge_Grade)


I have some bad news, floating point numbers are not real. 
No one can squeeze infinity into a finite number of bits.
People have to compromise, and it's ok as long as we are 
aware of the trade-offs and limitations.

After loosing several nights of sleep on a very nasty bug, 
it eventually boiled down to an equation having very big and
very small (close to zero) roots. These kind of problems are
easily solved, either by using a more stable solver, or by 
'bumping' up the floating point number accuracy.

So, I decided that all my math will now track their floating
point error and report it to me at the end. This way - I could
easily identify errors due to floating point accuracy.

The correct & robust solution, is to use Interval objects
instead of floating point objects (such as my 'Interval.h'
file located at https://github.com/DanIsraelMalta/NumericalPlusPlus,
or 'interval.m' located at https://github.com/DanIsraelMalta/Interval-arithmetic).
But many programmers don't really dig the whole interval stuff, and they would
rather change a float to a double if that would solve the problem.

This small module is for them.
It defines a small object called 'Real', which can perform the following:
1) It can be defined to be a float or a double, all by a compile time template.
2) If it is a float - it can track its error, relative to a double, using a
   simple compile time template.

This way, we can write our algorithm with 'floating point variables' and observe
their error, relative to double, at the end.
If the error is to big - just change the variable to a double. Walla...

For example, the following output:

```c
float arithmetics:
100.00001f * 100.00001f - 100.0f * 100.0f = {value = 0.001953, exact value = 0.001526, error = -0.000427}
(100.00001f + 100.0f) * (100.00001f - 100.0f) = {value = 0.001526, exact value = 0.001526, error = 0.0}

double arithmetics:
100.00001 * 100.00001 - 100.0 * 100.0 = 0.002
(100.00001 + 100.0) * (100.00001 - 100.0) = 0.002

float arithmetics: 1000000.0f + 1.2f - 1000000.0f = {value = 1.1857, exact value = 1.2, error = 0.0125}
double arithmetics: 1000000.0 + 1.2 - 1000000.0 = 1.2
```

originate from the following program:

```c
int main() {
    using FloatCompare = FP::Real<FP::Precision::Single, FP::Compare::Yes>;
    using Float = FP::Real<FP::Precision::Single>;
    using Double = FP::Real<FP::Precision::Double>;

    // test accuracy loss when calculating difference between the square of two values which are almost identical
    {
        FloatCompare c(100.00001f),
                     d(100.0f),
                     e = c * c - d * d,         // introduces numerical error
                     f = (c + d) * (c - d);     // numerical error is canceled
        std::cout << "float arithmetics:\n 100.00001f * 100.00001f - 100.0f * 100.0f = " << e << "\n (100.00001f + 100.0f) * (100.00001f - 100.0f) = " << f << "\n";

        Double g(100.00001),
               h(100.0),
               i = g * g - h * h,           // introduces numerical error
               j = (g + h) * (g - h);       // numerical error is canceled
        std::cout << "double arithmetics:\n  100.00001 * 100.00001 - 100.0 * 100.0 = " << i << "\n  (100.00001 + 100.0) * (100.00001 - 100.0) = " << j << "\n\n\n";
    }

    // test accuracy distributive arithmetics
    {
        FloatCompare a(1000000.0f),
                     b(1.2f),
                     one = a + b - a;
        std::cout << "float arithmetics: 1000000.0f + 1.2f - 1000000.0f = " << one << "\n";

        Double c(1000000.0),
               d(1.2),
               two = c + d - c;
        std::cout << "double arithmetics: 1000000.0 + 1.2 - 1000000.0 = " << two << "\n";
    }

  return 1;
}
