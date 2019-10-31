#include "diffutils.h"


// Function to display the differences between two vector<uint8_t>s
void DiffUtils::diff(vector<uint8_t> X, vector<uint8_t> Y, int m, int n)
{
    // if last character of X and Y matches
    if (m > 0 && n > 0 && X[m - 1] == Y[n - 1])
    {
        diff(X, Y, m - 1, n - 1);
    }

    // current character of Y is not present in X
    else if (n > 0 && (m == 0 || lookup[m][n - 1] >= lookup[m - 1][n]))
    {
        diff(X, Y, m, n - 1);
        this->added_bytes.push_back(pair<unsigned long, uint8_t>(n - 1, Y[n - 1]));
    }

    // current character of X is not present in Y
    else if (m > 0 && (n == 0 || lookup[m][n - 1] < lookup[m - 1][n]))
    {
        diff(X, Y, m - 1, n);
        this->deleted_bytes.push_back(pair<unsigned long, uint8_t>(m - 1, X[m - 1]));
    }
}

// Function to fill lookup table by finding the length of LCS
// of subvector<uint8_t> X[0..m-1] and Y[0..n-1]
void DiffUtils::lcs_length(vector<uint8_t> X, vector<uint8_t> Y, int m, int n)
{
    // first column of the lookup table will be all 0
    for (int i = 0; i <= m; i++)
        lookup[i][0] = 0;

    // first row of the lookup table will be all 0
    for (int j = 0; j <= n; j++)
        lookup[0][j] = 0;

    // fill the lookup table in bottom-up manner
    for (int i = 1; i <= m; i++)
    {
        for (int j = 1; j <= n; j++)
        {
            // if current character of X and Y matches
            if (X[i - 1] == Y[j - 1])
                lookup[i][j] = lookup[i - 1][j - 1] + 1;

            // else if current character of X and Y don't match
            else
                lookup[i][j] = max(lookup[i - 1][j], lookup[i][j - 1]);
        }
    }
}
