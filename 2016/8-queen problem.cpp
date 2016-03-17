bool in_range(int x, int low_bound, int high_bound)
{
	if (x < low_bound) return false;
	if (x > high_bound) return false;

	return true;
}

// x should be ensured at least with the size of n
int dfs(int x[], int n, int depth)
{
	if (depth == n) return 1;

	// note true stands for invalid place
	// so we initialize bitmap with all falses
	bool *bitmap = new bool[n] {};
	// mark invalid places
	for (int i = 0; i < depth; ++i)
	{
		bitmap[x[i]] = true;

		int delta = depth - i;
		if (in_range(x[i] - delta, 0, n - 1))
			bitmap[x[i] - delta] = true;
		if (in_range(x[i] + delta, 0, n - 1))
			bitmap[x[i] + delta] = true;
	}

	// search downward the tree
	int kase = 0;
	for (int i = 0; i < n; ++i)
	{
		x[depth] = i;
		kase += bitmap[i] ? 0 : dfs(x, n, depth + 1);
	}

	delete bitmap;
	return kase;
}