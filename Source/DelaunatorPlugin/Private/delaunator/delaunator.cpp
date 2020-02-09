#include "delaunator/delaunator.hpp"

namespace delaunator
{

Delaunator::Delaunator()
    : coords(),
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
}

Delaunator::Delaunator(const FReal* in_coords, FIndex in_coords_num)
    : coords(in_coords, in_coords_num),
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

void Delaunator::update(const TArray<FVector2D>& InPoints)
{
    if (InPoints.Num() >= 3)
    {
        coords = MakeArrayView(
            reinterpret_cast<const float*>(InPoints.GetData()),
            InPoints.Num()*2
            );

        update();
    }
}

void Delaunator::update()
{
    FIndex n = coords.Num() >> 1;

    FReal max_x = TNumericLimits<FReal>::Min();
    FReal max_y = TNumericLimits<FReal>::Min();
    FReal min_x = TNumericLimits<FReal>::Max();
    FReal min_y = TNumericLimits<FReal>::Max();

    TArray<FIndex> ids;
    ids.SetNumUninitialized(n);

    for (FIndex i=0; i<n; ++i)
    {
        const FReal x = coords[2 * i];
        const FReal y = coords[2 * i + 1];

        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
        if (x > max_x) max_x = x;
        if (y > max_y) max_y = y;

        ids[i] = i;
    }
    const FReal cx = (min_x + max_x) / 2;
    const FReal cy = (min_y + max_y) / 2;
    FReal min_dist = TNumericLimits<FReal>::Max();

    FIndex i0 = INVALID_INDEX;
    FIndex i1 = INVALID_INDEX;
    FIndex i2 = INVALID_INDEX;

    // pick a seed point close to the centroid

    for (FIndex i = 0; i < n; i++)
    {
        const FReal d = dist(cx, cy, coords[2 * i], coords[2 * i + 1]);

        if (d < min_dist)
        {
            i0 = i;
            min_dist = d;
        }
    }

    const FReal i0x = coords[2 * i0];
    const FReal i0y = coords[2 * i0 + 1];

    min_dist = TNumericLimits<FReal>::Max();

    // find the point closest to the seed

    for (FIndex i = 0; i < n; i++)
    {
        if (i == i0) continue;
        const FReal d = dist(i0x, i0y, coords[2 * i], coords[2 * i + 1]);
        if (d < min_dist && d > 0.0)
        {
            i1 = i;
            min_dist = d;
        }
    }

    FReal i1x = coords[2 * i1];
    FReal i1y = coords[2 * i1 + 1];

    FReal min_radius = TNumericLimits<FReal>::Max();

    // find the third point which forms
    // the smallest circumcircle with the first two

    for (FIndex i = 0; i < n; i++)
    {
        if (i == i0 || i == i1) continue;

        const FReal r = circumradius(
            i0x, i0y, i1x, i1y, coords[2 * i], coords[2 * i + 1]);

        if (r < min_radius)
        {
            i2 = i;
            min_radius = r;
        }
    }

    check(min_radius < TNumericLimits<FReal>::Max());

    FReal i2x = coords[2 * i2];
    FReal i2y = coords[2 * i2 + 1];

    // swap the order of the seed points for counter-clockwise orientation
    if (orient(i0x, i0y, i1x, i1y, i2x, i2y))
    {
        Swap(i1, i2);
        Swap(i1x, i2x);
        Swap(i1y, i2y);
    }

    // sort the points by distance from the seed triangle circumcenter

    circumcenter(m_centerx, m_centery, i0x, i0y, i1x, i1y, i2x, i2y);

    ids.Sort(compare(coords, m_centerx, m_centery));

    // initialize a hash table for storing edges of the advancing convex hull
    m_hash_size = FMath::CeilToInt(FMath::Sqrt(n));
    m_hash.SetNumUninitialized(m_hash_size);
    FMemory::Memset(m_hash.GetData(), ~0, m_hash_size*m_hash.GetTypeSize());

    // initialize arrays for tracking the edges of the advancing convex hull
    hull_prev.SetNumUninitialized(n);
    hull_next.SetNumUninitialized(n);
    hull_tri.SetNumUninitialized(n);

    // set up the seed triangle as the starting hull

    hull_start = i0;
    hull_size = 3;

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
    triangles.Reset(max_triangles * 3);
    halfedges.Reset(max_triangles * 3);

    add_triangle(i0, i1, i2, INVALID_INDEX, INVALID_INDEX, INVALID_INDEX);

    FReal xp = BIG_NUMBER; // = std::numeric_limits<FReal>::quiet_NaN();
    FReal yp = BIG_NUMBER; // = std::numeric_limits<FReal>::quiet_NaN();

    for (FIndex k = 0; k < n; k++)
    {
        const FIndex i = ids[k];
        const FReal x = coords[2 * i];
        const FReal y = coords[2 * i + 1];

        // skip near-duplicate points
        if (k > 0 && check_pts_equal(x, y, xp, yp))
        {
            continue;
        }

        xp = x;
        yp = y;

        // skip seed triangle points
        if (check_pts_equal(x, y, i0x, i0y) ||
            check_pts_equal(x, y, i1x, i1y) ||
            check_pts_equal(x, y, i2x, i2y))
        {
            continue;
        }

        // find a visible edge on the convex hull using edge hash

        FIndex start = 0;
        FIndex key = hash_key(x, y);

        for (FIndex j = 0; j < m_hash_size; j++)
        {
            start = m_hash[fast_mod(key + j, m_hash_size)];

            if (start != INVALID_INDEX && start != hull_next[start])
            {
                break;
            }
        }

        start = hull_prev[start];
        FIndex e = start;
        FIndex q;

        while (
            q = hull_next[e],
            !orient(x, y, coords[2 * e], coords[2 * e + 1], coords[2 * q], coords[2 * q + 1]))
        {
            e = q;

            if (e == start)
            {
                e = INVALID_INDEX;
                break;
            }
        }

        if (e == INVALID_INDEX)
        {
            continue; // likely a near-duplicate point; skip it
        }

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

        // walk forward through the hull,
        // adding more triangles and flipping recursively
        FIndex next = hull_next[e];
        while (
            q = hull_next[next],
            orient(x, y, coords[2 * next], coords[2 * next + 1], coords[2 * q], coords[2 * q + 1]))
        {
            t = add_triangle(next, i, q, hull_tri[i], INVALID_INDEX, hull_tri[next]);
            hull_tri[i] = legalize(t + 2);
            hull_next[next] = next; // mark as removed
            hull_size--;
            next = q;
        }

        // walk backward from the other side,
        // adding more triangles and flipping
        if (e == start)
        {
            while (
                q = hull_prev[e],
                orient(x, y, coords[2 * q], coords[2 * q + 1], coords[2 * e], coords[2 * e + 1]))
            {
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

    // trim allocation sizes
    triangles.Shrink();
    halfedges.Shrink();
}

FIndex Delaunator::legalize(FIndex a)
{
    FIndex i = 0;
    FIndex ar = 0;
    m_edge_stack.Reset();

    // recursion eliminated with a fixed-size stack
    while (true)
    {
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

        if (b == INVALID_INDEX)
        {
            if (i > 0)
            {
                i--;
                a = m_edge_stack[i];
                continue;
            }
            else
            {
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

        if (illegal)
        {
            triangles[a] = p1;
            triangles[b] = p0;

            auto hbl = halfedges[bl];

            // edge swapped on the other side of the hull (rare);
            // fix the halfedge reference
            if (hbl == INVALID_INDEX)
            {
                FIndex e = hull_start;
                do
                {
                    if (hull_tri[e] == bl)
                    {
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

            if (i < m_edge_stack.Num())
            {
                m_edge_stack[i] = br;
            }
            else
            {
                m_edge_stack.Emplace(br);
            }
            i++;

        }
        else
        {
            if (i > 0)
            {
                i--;
                a = m_edge_stack[i];
                continue;
            }
            else
            {
                break;
            }
        }
    }
    return ar;
}

} //namespace delaunator
