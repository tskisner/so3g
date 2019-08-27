import numpy as np
from spt3g import coordinateutils as cu3g

"""We are using the spt3g quaternion containers,
i.e. cu3g.G3VectorQuat and cu3g.quat.  One way these are nice is that
they have accelerated quaternion arithmetic with operator overloading.
The component ordering is (1,i,j,k).  We are restricted to 0d or 1d,
and that's fine."""

DEG = np.pi/180

def euler(axis, angle):
    """
    The quaternion representing of an Euler rotation.

    For example, if axis=2 the computed quaternion(s) will have
    components:

      q = (cos(angle/2), 0, 0, sin(angle/2))

    Parameters
    ----------
    axis : {0, 1, 2}
        The index of the cartesian axis of the rotation (x, y, z).
    angle : float or 1-d float array
        Angle of rotation, in radians.

    Returns
    -------
    quat or G3VectorQuat, depending on ndim(angle).
    """
    # Either angle or axis or both can be vectors.
    angle = np.asarray(angle)
    shape = np.broadcast(axis, angle).shape + (4,)
    c, s = np.cos(angle/2), np.sin(angle/2)
    q = np.zeros(shape)
    q[...,0] = c
    q[...,axis+1] = s
    if len(shape) == 1:
        return cu3g.quat(*q)
    return cu3g.G3VectorQuat(q)

def rotation_iso(theta, phi, gamma=None):
    """Returns the quaternion that composes the Euler rotations:

        Qz(phi) Qy(theta) Qz(gamma)

    Note arguments are in radians.
    """
    output = euler(2, phi) * euler(1, theta)
    if gamma is None:
        return output
    return output * euler(2, gamma)

def rotation_lonlat(lon, lat, gamma=0.):
    """Returns the quaternion that composes the Euler rotations:

        Qz(lon) Qy(pi/2 - lat) Qz(gamma)

    Note arguments are in radians.
    """
    return rotation_iso(np.pi/2 - lat, lon, gamma)

def decompose_iso(q):
    """Decomposes the rotation encoded by q into the product of Euler
    rotations:

        q = Qz(phi) Qy(-theta) Qz(gamma)

    and returns theta, phi, gamma.  Why that order?  Because ISO.

    Parameters
    ----------
    q : quat or G3VectorQuat
        The quaternion(s) to be decomposed.

    Returns
    -------
    (theta, phi, gamma) : tuple of floats or of 1-d arrays
        The rotation angles, in radians.
    """

    if isinstance(q, cu3g.quat):
        a,b,c,d = q.a, q.b, q.c, q.d
    else:
        a,b,c,d = np.transpose(q)

    gamma = np.arctan2(a*b+c*d, a*c-b*d)
    phi = np.arctan2(c*d-a*b, a*c+b*d)

    # There are many ways to get theta; this is probably the most
    # expensive but the most robust.
    theta = 2 * np.arctan2((b**2 + c**2)**.5, (a**2 + d**2)**.5)

    return (theta, phi, gamma)

def decompose_lonlat(q):
    """Like decompose_iso, but returns (lon, lat, gamma)."""
    phi, theta, gamma = decompose_iso(q)
    return (theta, np.pi/2 - phi, gamma)
