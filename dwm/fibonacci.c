void
fibonacci(Monitor *mon, int s) {
	unsigned int i, n, nx, ny, nw, nh;
	Client *c;

	for(n = 0, c = nexttiled(mon->clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;
	
	// Initialize with gaps
	nx = mon->wx + mon->gappoh;
	ny = mon->wy + mon->gappov;
	nw = mon->ww - 2 * mon->gappoh;
	nh = mon->wh - 2 * mon->gappov;
	
	for(i = 0, c = nexttiled(mon->clients); c; c = nexttiled(c->next)) {
		if((i % 2 && nh / 2 > 2 * c->bw + mon->gappiv)
		   || (!(i % 2) && nw / 2 > 2 * c->bw + mon->gappih)) {
			if(i < n - 1) {
				if(i % 2) {
					nh = (nh - mon->gappiv) / 2;
				} else {
					nw = (nw - mon->gappih) / 2;
				}
				if((i % 4) == 2 && !s)
					nx += nw + mon->gappih;
				else if((i % 4) == 3 && !s)
					ny += nh + mon->gappiv;
			}
			if((i % 4) == 0) {
				if(s)
					ny += nh + mon->gappiv;
				else
					ny -= nh + mon->gappiv;
			}
			else if((i % 4) == 1)
				nx += nw + mon->gappih;
			else if((i % 4) == 2)
				ny += nh + mon->gappiv;
			else if((i % 4) == 3) {
				if(s)
					nx += nw + mon->gappih;
				else
					nx -= nw + mon->gappih;
			}
			if(i == 0)
			{
				if(n != 1)
					nw = (mon->ww - 2 * mon->gappoh) * mon->mfact;
				ny = mon->wy + mon->gappov;
			}
			else if(i == 1)
				nw = mon->ww - nw - 2 * mon->gappoh - mon->gappih;
			i++;
		}
		// Apply gaps in resize
		resize(c, nx, ny, nw - 2 * c->bw - mon->gappih, nh - 2 * c->bw - mon->gappiv, False);
	}
}

void
dwindle(Monitor *mon) {
	fibonacci(mon, 1);
}

void
spiral(Monitor *mon) {
	fibonacci(mon, 0);
}
