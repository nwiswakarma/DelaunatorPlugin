
#include "delaunator/delaunator.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <tuple>

namespace delaunator {

//@see https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op/33333636#33333636
inline FIndex fast_mod(const FIndex i, const FIndex c) {
    return i >= c ? i % c : i;
}

// Kahan and Babuska summation, Neumaier variant; accumulates less FP error
inline FReal sum(const TArray<FReal>& x) {
    FReal sum = x[0];
    FReal err = 0.0;

    for (FIndex i = 1; i < x.Num(); i++) {
        const FReal k = x[i];
        const FReal m = sum + k;
        err += std::fabs(sum) >= std::fabs(k) ? sum - m + k : k - m + sum;
        sum = m;
    }
    return sum + err;
}

inline FReal dist(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by) {
    const FReal dx = ax - bx;
    const FReal dy = ay - by;
    return dx * dx + dy * dy;
}

inline FReal circumradius(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy) {
    const FReal dx = bx - ax;
    const FReal dy = by - ay;
    const FReal ex = cx - ax;
    const FReal ey = cy - ay;

    const FReal bl = dx * dx + dy * dy;
    const FReal cl = ex * ex + ey * ey;
    const FReal d = dx * ey - dy * ex;

    const FReal x = (ey * bl - dy * cl) * 0.5 / d;
    const FReal y = (dx * cl - ex * bl) * 0.5 / d;

    if ((bl > 0.0 || bl < 0.0) && (cl > 0.0 || cl < 0.0) && (d > 0.0 || d < 0.0)) {
        return x * x + y * y;
    } else {
        return (std::numeric_limits<FReal>::max)();
    }
}

inline bool orient(
    const FReal px,
    const FReal py,
    const FReal qx,
    const FReal qy,
    const FReal rx,
    const FReal ry) {
    return (qy - py) * (rx - qx) - (qx - px) * (ry - qy) < 0.0;
}

inline void circumcenter(
    FReal& centerx,
    FReal& centery,
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy) {
    const FReal dx = bx - ax;
    const FReal dy = by - ay;
    const FReal ex = cx - ax;
    const FReal ey = cy - ay;

    const FReal bl = dx * dx + dy * dy;
    const FReal cl = ex * ex + ey * ey;
    const FReal d = dx * ey - dy * ex;

    const FReal x = ax + (ey * bl - dy * cl) * 0.5 / d;
    const FReal y = ay + (dx * cl - ex * bl) * 0.5 / d;

    centerx = x;
    centery = y;
}


struct compare
{
    //const FReal* coords;
    //const FIndex coords_num;
    TArrayView<const FReal> coords;
    TArray<FReal> dists;

   // compare(const FReal* coords, FIndex coords_num, FReal centerx, FReal centery)
    compare(const TArrayView<const FReal>& in_coords, FReal centerx, FReal centery)
        : coords(in_coords)
        //, coords_num(coords_num)
    {
        //FIndex n = coords_num / 2;
        FIndex n = coords.Num() / 2;
        //dists.reserve(n);
        dists.Reserve(n);
        FReal const *xcoord = coords.GetData();
        FReal const *ycoord = coords.GetData()+1;
        while (n--)
        {
            dists.Emplace(dist(*xcoord, *ycoord, centerx, centery));
            xcoord += 2;
            ycoord += 2;
        }
    }

    bool operator()(FIndex i, FIndex j) const
    {
        const FReal diff1 = dists[i] - dists[j];
        const FReal diff2 = coords[2 * i] - coords[2 * j];
        const FReal diff3 = coords[2 * i + 1] - coords[2 * j + 1];

        //ABELL - Not sure why we're not just checking != 0 here.
        if (diff1 > 0.0 || diff1 < 0.0) {
            return diff1 < 0;
        } else if (diff2 > 0.0 || diff2 < 0.0) {
            return diff2 < 0;
        } else {
            return diff3 < 0;
        }
    }
};


inline bool in_circle(
    const FReal ax,
    const FReal ay,
    const FReal bx,
    const FReal by,
    const FReal cx,
    const FReal cy,
    const FReal px,
    const FReal py) {
    const FReal dx = ax - px;
    const FReal dy = ay - py;
    const FReal ex = bx - px;
    const FReal ey = by - py;
    const FReal fx = cx - px;
    const FReal fy = cy - py;

    const FReal ap = dx * dx + dy * dy;
    const FReal bp = ex * ex + ey * ey;
    const FReal cp = fx * fx + fy * fy;

    return (dx * (ey * cp - bp * fy) -
            dy * (ex * cp - bp * fx) +
            ap * (ex * fy - ey * fx)) < 0.0;
}

constexpr FReal EPSILON = std::numeric_limits<FReal>::epsilon();

inline bool check_pts_equal(FReal x1, FReal y1, FReal x2, FReal y2) {
    return std::fabs(x1 - x2) <= EPSILON &&
           std::fabs(y1 - y2) <= EPSILON;
}

// monotonically increases with real angle, but doesn't need expensive trigonometry
inline FReal pseudo_angle(const FReal dx, const FReal dy) {
    const FReal p = dx / (std::abs(dx) + std::abs(dy));
    return (dy > 0.0 ? 3.0 - p : 1.0 + p) / 4.0; // [0..1)
}

struct DelaunatorPoint {
    FIndex i;
    FReal x;
    FReal y;
    FIndex t;
    FIndex prev;
    FIndex next;
    bool removed;
};

Delaunator::Delaunator(const FReal* in_coords, FIndex in_coords_num)
    : coords(in_coords, in_coords_num),
      //coords_num(in_coords_num),
      triangles(),
      halfedges(),
      hull_prev(),
      hull_next(),
      hull_tri(),
      hull_start(),
      m_hash(),
      m_hash_size(),
      m_edge_stack()
{
    update();
}

void Delaunator::update()
{
    //FIndex n = coords_num >> 1;
    FIndex n = coords.Num() >> 1;

    FReal max_x = (std::numeric_limits<FReal>::min)();
    FReal max_y = (std::numeric_limits<FReal>::min)();
    FReal min_x = (std::numeric_limits<FReal>::max)();
    FReal min_y = (std::numeric_limits<FReal>::max)();

    TArray<FIndex> ids;
    //ids.reserve(n);
    ids.Reserve(n);

    for (FIndex i = 0; i < n; i++) {
        const FReal x = coords[2 * i];
        const FReal y = coords[2 * i + 1];

        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
        if (x > max_x) max_x = x;
        if (y > max_y) max_y = y;

        ids.Emplace(i);
    }
    const FReal cx = (min_x + max_x) / 2;
    const FReal cy = (min_y + max_y) / 2;
    FReal min_dist = (std::numeric_limits<FReal>::max)();

    FIndex i0 = INVALID_INDEX;
    FIndex i1 = INVALID_INDEX;
    FIndex i2 = INVALID_INDEX;

    // pick a seed point close to the centroid
    for (FIndex i = 0; i < n; i++) {
        const FReal d = dist(cx, cy, coords[2 * i], coords[2 * i + 1]);
        if (d < min_dist) {
            i0 = i;
            min_dist = d;
        }
    }

    const FReal i0x = coords[2 * i0];
    const FReal i0y = coords[2 * i0 + 1];

    min_dist = (std::numeric_limits<FReal>::max)();

    // find the point closest to the seed
    for (FIndex i = 0; i < n; i++) {
        if (i == i0) continue;
        const FReal d = dist(i0x, i0y, coords[2 * i], coords[2 * i + 1]);
        if (d < min_dist && d > 0.0) {
            i1 = i;
            min_dist = d;
        }
    }

    FReal i1x = coords[2 * i1];
    FReal i1y = coords[2 * i1 + 1];

    FReal min_radius = (std::numeric_limits<FReal>::max)();

    // find the third point which forms the smallest circumcircle with the first two
    for (FIndex i = 0; i < n; i++) {
        if (i == i0 || i == i1) continue;

        const FReal r = circumradius(
            i0x, i0y, i1x, i1y, coords[2 * i], coords[2 * i + 1]);

        if (r < min_radius) {
            i2 = i;
            min_radius = r;
        }
    }

    if (!(min_radius < (std::numeric_limits<FReal>::max()))) {
        throw std::runtime_error("not triangulation");
    }

    FReal i2x = coords[2 * i2];
    FReal i2y = coords[2 * i2 + 1];

    if (orient(i0x, i0y, i1x, i1y, i2x, i2y)) {
        std::swap(i1, i2);
        std::swap(i1x, i2x);
        std::swap(i1y, i2y);
    }

    circumcenter(m_centerx, m_centery, i0x, i0y, i1x, i1y, i2x, i2y);

    // sort the points by distance from the seed triangle circumcenter
    //std::sort(ids.begin(), ids.end(), compare{ coords, coords_num, m_centerx, m_centery });
    compare compare_inst(coords, m_centerx, m_centery);
    ids.Sort(compare_inst);

    // initialize a hash table for storing edges of the advancing convex hull
    m_hash_size = static_cast<FIndex>(std::llround(std::ceil(std::sqrt(n))));
    m_hash.SetNumUninitialized(m_hash_size);
    //std::fill(m_hash.begin(), m_hash.end(), INVALID_INDEX);
    m_hash.Init(INVALID_INDEX, m_hash_size);

    // initialize arrays for tracking the edges of the advancing convex hull
    //hull_prev.resize(n);
    //hull_next.resize(n);
    //hull_tri.resize(n);
    hull_prev.SetNumUninitialized(n);
    hull_next.SetNumUninitialized(n);
    hull_tri.SetNumUninitialized(n);

    hull_start = i0;

    FIndex hull_size = 3;

    hull_next[i0] = hull_prev[i2] = i1;
    hull_next[i1] = hull_prev[i0] = i2;
    hull_next[i2] = hull_prev[i1] = i0;

    hull_tri[i0] = 0;
    hull_tri[i1] = 1;
    hull_tri[i2] = 2;

    m_hash[hash_key(i0x, i0y)] = i0;
    m_hash[hash_key(i1x, i1y)] = i1;
    m_hash[hash_key(i2x, i2y)] = i2;

    FIndex max_triangles = n < 3 ? 1 : 2 * n - 5;
    //triangles.reserve(max_triangles * 3);
    //halfedges.reserve(max_triangles * 3);
    triangles.Reserve(max_triangles * 3);
    halfedges.Reserve(max_triangles * 3);
    add_triangle(i0, i1, i2, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX);
    FReal xp = std::numeric_limits<FReal>::quiet_NaN();
    FReal yp = std::numeric_limits<FReal>::quiet_NaN();
    for (FIndex k = 0; k < n; k++) {
        const FIndex i = ids[k];
        const FReal x = coords[2 * i];
        const FReal y = coords[2 * i + 1];

        // skip near-duplicate points
        if (k > 0 && check_pts_equal(x, y, xp, yp)) continue;
        xp = x;
        yp = y;

        // skip seed triangle points
        if (
            check_pts_equal(x, y, i0x, i0y) ||
            check_pts_equal(x, y, i1x, i1y) ||
            check_pts_equal(x, y, i2x, i2y)) continue;

        // find a visible edge on the convex hull using edge hash
        FIndex start = 0;

        FIndex key = hash_key(x, y);
        for (FIndex j = 0; j < m_hash_size; j++) {
            start = m_hash[fast_mod(key + j, m_hash_size)];
            if (start != INVALID_INDEX && start != hull_next[start]) break;
        }

        start = hull_prev[start];
        FIndex e = start;
        FIndex q;

        while (q = hull_next[e], !orient(x, y, coords[2 * e], coords[2 * e + 1], coords[2 * q], coords[2 * q + 1])) { //TODO: does it works in a same way as in JS
            e = q;
            if (e == start) {
                e = INVALID_INDEX;
                break;
            }
        }

        if (e == INVALID_INDEX) continue; // likely a near-duplicate point; skip it

        // add the first triangle from the point
        FIndex t = add_triangle(
            e,
            i,
            hull_next[e],
            INVALID_INDEX,
            INVALID_INDEX,
            hull_tri[e]);

        hull_tri[i] = legalize(t + 2);
        hull_tri[e] = t;
        hull_size++;

        // walk forward through the hull, adding more triangles and flipping recursively
        FIndex next = hull_next[e];
        while (
            q = hull_next[next],
            orient(x, y, coords[2 * next], coords[2 * next + 1], coords[2 * q], coords[2 * q + 1])) {
            t = add_triangle(next, i, q, hull_tri[i], INVALID_INDEX, hull_tri[next]);
            hull_tri[i] = legalize(t + 2);
            hull_next[next] = next; // mark as removed
            hull_size--;
            next = q;
        }

        // walk backward from the other side, adding more triangles and flipping
        if (e == start) {
            while (
                q = hull_prev[e],
                orient(x, y, coords[2 * q], coords[2 * q + 1], coords[2 * e], coords[2 * e + 1])) {
                t = add_triangle(q, i, e, INVALID_INDEX, hull_tri[e], hull_tri[q]);
                legalize(t + 2);
                hull_tri[q] = t;
                hull_next[e] = e; // mark as removed
                hull_size--;
                e = q;
            }
        }

        // update the hull indices
        hull_prev[i] = e;
        hull_start = e;
        hull_prev[next] = i;
        hull_next[e] = i;
        hull_next[i] = next;

        m_hash[hash_key(x, y)] = i;
        m_hash[hash_key(coords[2 * e], coords[2 * e + 1])] = e;
    }
}

FReal Delaunator::get_hull_area() {
    TArray<FReal> hull_area;
    FIndex e = hull_start;
    do {
        hull_area.Emplace((coords[2 * e] - coords[2 * hull_prev[e]]) * (coords[2 * e + 1] + coords[2 * hull_prev[e] + 1]));
        e = hull_next[e];
    } while (e != hull_start);
    return sum(hull_area);
}

FIndex Delaunator::legalize(FIndex a) {
    FIndex i = 0;
    FIndex ar = 0;
    m_edge_stack.Reset();

    // recursion eliminated with a fixed-size stack
    while (true) {
        const FIndex b = halfedges[a];

        /* if the pair of triangles doesn't satisfy the Delaunay condition
        * (p1 is inside the circumcircle of [p0, pl, pr]), flip them,
        * then do the same check/flip recursively for the new pair of triangles
        *
        *           pl                    pl
        *          /||\                  /  \
        *       al/ || \bl            al/    \a
        *        /  ||  \              /      \
        *       /  a||b  \    flip    /___ar___\
        *     p0\   ||   /p1   =>   p0\---bl---/p1
        *        \  ||  /              \      /
        *       ar\ || /br             b\    /br
        *          \||/                  \  /
        *           pr                    pr
        */
        const FIndex a0 = 3 * (a / 3);
        ar = a0 + (a + 2) % 3;

        if (b == INVALID_INDEX) {
            if (i > 0) {
                i--;
                a = m_edge_stack[i];
                continue;
            } else {
                //i = INVALID_INDEX;
                break;
            }
        }

        const FIndex b0 = 3 * (b / 3);
        const FIndex al = a0 + (a + 1) % 3;
        const FIndex bl = b0 + (b + 2) % 3;

        const FIndex p0 = triangles[ar];
        const FIndex pr = triangles[a];
        const FIndex pl = triangles[al];
        const FIndex p1 = triangles[bl];

        const bool illegal = in_circle(
            coords[2 * p0],
            coords[2 * p0 + 1],
            coords[2 * pr],
            coords[2 * pr + 1],
            coords[2 * pl],
            coords[2 * pl + 1],
            coords[2 * p1],
            coords[2 * p1 + 1]);

        if (illegal) {
            triangles[a] = p1;
            triangles[b] = p0;

            auto hbl = halfedges[bl];

            // edge swapped on the other side of the hull (rare); fix the halfedge reference
            if (hbl == INVALID_INDEX) {
                FIndex e = hull_start;
                do {
                    if (hull_tri[e] == bl) {
                        hull_tri[e] = a;
                        break;
                    }
                    e = hull_prev[e];
                } while (e != hull_start);
            }
            link(a, hbl);
            link(b, halfedges[ar]);
            link(ar, bl);
            FIndex br = b0 + (b + 1) % 3;

            if (i < m_edge_stack.Num()) {
                m_edge_stack[i] = br;
            } else {
                m_edge_stack.Emplace(br);
            }
            i++;

        } else {
            if (i > 0) {
                i--;
                a = m_edge_stack[i];
                continue;
            } else {
                break;
            }
        }
    }
    return ar;
}

FIndex Delaunator::hash_key(const FReal x, const FReal y) const {
    const FReal dx = x - m_centerx;
    const FReal dy = y - m_centery;
    return fast_mod(
        static_cast<FIndex>(std::llround(std::floor(pseudo_angle(dx, dy) * static_cast<FReal>(m_hash_size)))),
        m_hash_size);
}

FIndex Delaunator::add_triangle(
    FIndex i0,
    FIndex i1,
    FIndex i2,
    FIndex a,
    FIndex b,
    FIndex c) {
    FIndex t = triangles.Num();
    triangles.Emplace(i0);
    triangles.Emplace(i1);
    triangles.Emplace(i2);
    link(t, a);
    link(t + 1, b);
    link(t + 2, c);
    return t;
}

void Delaunator::link(const FIndex a, const FIndex b) {
    FIndex s = halfedges.Num();
    if (a == s) {
        halfedges.Emplace(b);
    } else if (a < s) {
        halfedges[a] = b;
    } else {
        throw std::runtime_error("Cannot link edge");
    }
    if (b != INVALID_INDEX) {
        FIndex s2 = halfedges.Num();
        if (b == s2) {
            halfedges.Emplace(a);
        } else if (b < s2) {
            halfedges[b] = a;
        } else {
            throw std::runtime_error("Cannot link edge");
        }
    }
}

} //namespace delaunator
