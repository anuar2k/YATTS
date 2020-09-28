export default [
  {
    id: 'channel',
    name: 'Channel variables',
    description: 'These variables will be sent every tick',
    groups: [
      {
        id: 'truck',
        name: 'Truck',
        variables: [
          {
            id: 'speed',
            type: 5
          },
          {
            id: 'blinker.left',
            type: 1
          }
        ]
      },
      {
        id: 'trailer',
        name: 'Trailer',
        variables: [
          {
            id: 'connected',
            type: 1
          },
          {
            id: 'wheel.speed',
            type: 5
          }
        ]
      }
    ]
  },
  {
    id: 'config',
    name: 'Config variables',
    description: 'These variables will be sent only upon change',
    groups: [
      {
        id: 'truck',
        name: 'Truck',
        variables: [
          {
            id: 'id',
            type: 15
          },
          {
            id: 'registration.plate',
            type: 15
          }
        ]
      }
    ]
  }
]
