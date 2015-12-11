/**
 * Heighway-Dragon fractal implementation.
 * This Range allows to browse through coordinates of a Heighway-Dragon fractal.
 *
 * It uses D Range component programming as it gives a very neat interface
 * to use that series. It specifically uses a Random-Access Range implementation
 * as the coordinate of a particular point may be given at BigO(log(n))
 * complexity time.
 */

/*
# Unit tests compilation:
dmd dragon.d -unittest -main -version=dragontest
*/
module dragon;


struct Dragon
{
private:
    ulong mIdx;

    enum Dir
    {
        up = 0, // Up direction by default
        left,
        down,
        right
    }

    /**
     * Rotate direction on the left.
     */
    Dir leftOf(Dir dir) const
    {
        return cast(Dir)((dir + 1) & 3);
    }


public:
    // 2D coordinates returned by the Dragon Range
    static struct Vector2
    {
        long x, y;

        /**
         * Mathematically:
         *   x = cos(order * PI / 4) * pow(sqrt(2), order)
         *   y = sin(order * PI / 4) * pow(sqrt(2), order)
         * It may be simplified as below.
         */
        Vector2 lastPoint(uint order) const
        {
            long n = 1L << (order >> 1);

            // 3 first bits gives a multiple of 45° angle
            switch (order & 7)
            {
            case 0:
                return Vector2( 0,  n);
            case 1:
                return Vector2( n,  n);
            case 2:
                return Vector2( n,  0);
            case 3:
                return Vector2( n, -n);
            case 4:
                return Vector2( 0, -n);
            case 5:
                return Vector2(-n, -n);
            case 6:
                return Vector2(-n,  0);
            case 7:
                return Vector2(-n,  n);
            default:
                assert(false);
            }
        }

        /**
         * The actual transformation is an iteration in the Dragon fractal.
         * It adds the rotated vector (depending of order) to the current
         * position.
         */
        void transform(Dir dir, uint order)
        {
            auto n = lastPoint(order);

            // simplify rotation matrix
            switch (dir)
            {
            case Dir.up:
                x += n.x;
                y += n.y;
                break;
            case Dir.left:
                x -= n.y;
                y += n.x;
                break;
            case Dir.down:
                x -= n.x;
                y -= n.y;
                break;
            case Dir.right:
                x += n.y;
                y -= n.x;
                break;
            default:
                assert(false);
            }
        }
    }

    /**
     * From a point index, get the minimal order of the fractal dichotomically.
     * Maths:
     * order = log2(ceil(idx))
     */
    auto getOrder(size_t idx = 0) const
    {
        ulong n = mIdx + idx;
        uint o = ulong.sizeof * 8 / 2;
        uint o2 = o >> 1;

        do
        {
            if ((1UL << o) < n)
                o += o2;
            else if ((1UL << (o - 1)) > n)
                o -= o2;
            else
                return o;
            o2 >>= 1;
        } while (o2 > 0);

        return o;
    }


    /* Range implementation */

    // Never empty so that makes it virtually an infinite range
    enum empty = false;

    this(ulong idx = 0)
    {
        mIdx = idx;
    }

    @property auto front() const
    {
        return opIndex(0);
    }

    void popFront()
    {
        assert(mIdx < ulong.max);
        mIdx++;
    }

    // used to make it a proper forward range
    @property auto save() const
    {
        return Dragon(mIdx);
    }

    /**
     * It implements Dragon look-up algorithm (BigO(log(n)) complexity).
     * From an index, it returns a 2D point coordinate.
     */
    auto opIndex(size_t idx) const
    {
        Dir     dir;
        Vector2 p;
        ulong   n = mIdx + idx;
        auto    o = getOrder(idx);    // get necessary order depending on n

        // special cases
        if (n == 0)
            return Vector2(0, 0);
        else if (n == 1)
            return Vector2(1, 0);

        // recursively
        while (n != (1UL << o))
        {
            if (n & (1UL << o))
            {
                // Apply transformation to the point
                p.transform(dir, o + 1);

                // Substract n from order
                n = (1UL << (o + 1)) - n;
                // Rotate direction on the left
                dir = leftOf(dir);
            }
            o--;
        }

        // Final transformation iteration
        p.transform(dir, o);

        return p;
    }
}

unittest
{
    // Basic non-regression test
    assert(Dragon()[500]            == Dragon.Vector2(18, 16));
    assert(Dragon()[7_000_000]      == Dragon.Vector2(-1280, 1592));
    assert(Dragon()[10_000_000_000] == Dragon.Vector2(90080, 48704));

    // Command-line arg test
    version (dragontest)
    {
        import core.runtime;
        import std.conv;
        import std.stdio;
        import std.string;

        auto cArgs = Runtime.cArgs; // C-style args from command-line

        if (cArgs.argc == 2)
        {
            auto d = Dragon();
            auto idx = cArgs.argv[1].fromStringz.to!size_t;

            // Random-access range thanks to opIndex() definition
            writeln(d[idx]);
        }
        else if (cArgs.argc == 3)
        {
            // Contruct with first command-line argument
            auto firstIdx = cArgs.argv[1].fromStringz.to!ulong;
            auto d = Dragon(firstIdx);
            auto n = cArgs.argv[2].fromStringz.to!int;

            // Loop through the Dragon range.
            // (input range thanks to front, popFront() and empty definitions)
            foreach (p; d)
            {
                if (--n < 0)
                    break;
                writeln(p);
            }
        }
        else
        {
            writeln("Usage - either display the coordinates of an index:\n"
                    "\tdragon <index>\n"
                    "... or displays a list of n points:\n"
                    "\tdragon <startindex> <n>");
        }
    }
}
