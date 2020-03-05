#pragma once

#include <limits>
#include <string>

/* Integer intervals 
 *   A simple interface using basic types that is optimized for clarity
 *   and not efficiency.
 */
namespace interval {

typedef std::pair<int,int> Interval;

// Least (minf) and greatest (pinf) interval bounds
const int minf = std::numeric_limits<int>::min();
const int pinf = std::numeric_limits<int>::max();

// Build and access data from intervals
Interval make(int l, int u);
int lower(Interval i);
int upper(Interval i);

// Interval describing all values
Interval full();

// Interval describing no values
Interval empty();

// Interval describing values in [0,1] range
Interval unit();

// Compute least upper bound of two intervals
Interval lub(Interval l, Interval r);

// Arithmetic operations on intervals
Interval neg(Interval i);
Interval add(Interval l, Interval r);
Interval sub(Interval l, Interval r);
Interval mul(Interval l, Interval r);
Interval div(Interval l, Interval r);

// Comparison operations on intervals
Interval lt(Interval l, Interval r);
Interval gt(Interval l, Interval r);
Interval eq(Interval l, Interval r);
Interval ne(Interval l, Interval r);

// Equality operations on intervals
bool operator==(Interval l, Interval r);
bool operator!=(Interval l, Interval r);

// Produce a printable representation of an interval
std::string str(Interval i);

} // end namespace interval
